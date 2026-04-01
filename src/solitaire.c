#include "solitaire.h"

#include <stdlib.h>
#include <time.h>

#define IN_BOX(x, y, b_x, b_y, b_w, b_h) (x >= (b_x) && x < (b_x) + (b_w) && y >= (b_y) && y < (b_y) + (b_h))
#define GET_STACK(board, field) (field == HAND ? &((Board *)board)->hand : &((Board *)board)->stacks[INDEX(field)])

int pick(DrawStack *stack) {
    stack->pick = 0;
    stack->flip = 255;
    stack->progression = 255;
    if(stack->revealed == 0) {
        return -1;
    }
    stack->revealed--;
    return stack->pile[stack->revealed];
}

void unpick(DrawStack *stack) {
    stack->revealed++;
}

void flip(DrawStack *stack) {
    stack->flip = 0;
    stack->pick = 255;
    stack->progression = 255;
    stack->recent_flip = 0;
    if(stack->from == stack->size) {
        stack->size = stack->revealed;
        stack->revealed = 0;
        stack->from = 0;
        return;
    }

    for(int i = 0; i < DRAW_SHOWN && stack->from < stack->size; i++) {
        stack->pile[stack->revealed] = stack->pile[stack->from];
        stack->recent_flip++;
        stack->revealed++;
        stack->from++;
    }
}

int getCard(Board *board) {
    if(board->cards_left == 0) {
        return -1;
    }
    uint8_t choice = rand() % board->cards_left;
    board->cards_left--;

    card card = board->order[choice];
    board->order[choice] = board->order[board->cards_left];
    board->order[board->cards_left] = card;
    return card;
}

void newCard(CardStack *stack, Board *board) {
    if(board->cards_left == 0 || stack->card_count == MAX_CARD_STACK) {
        return;
    }
    stack->cards[stack->card_count] = getCard(board);
    stack->card_count++;
}

void exposeCard(CardStack *stack, Board *board) {
    if(stack->upside_down == 0) {
        return;
    }
    stack->upside_down--;
    stack->flip = 0;
    int new_card = getCard(board);
    if(new_card != -1) {
        stack->cards[0] = new_card;
        stack->card_count = 1;
    }
}

void moveCards(CardStack *from, CardStack *to, uint8_t from_index) {
    uint8_t to_move = from->card_count - from_index;
    if(from_index >= from->card_count || to_move > MAX_CARD_STACK - to->card_count || !from || !to) {
        return;
    }
    for(uint8_t i = 0; i < to_move; i++) {
        to->cards[to->card_count + i] = from->cards[i + from_index];
    }
    to->card_count += to_move;
    from->card_count -= to_move;
}

int moveCardsGeneral(Board *board, cardloc from, cardloc to, uint8_t count) {
    // Ensure that if `to` or `from` is anything other than a stack, you aren't moving more than one card.
    if(count != 1 && (!IS_TYPE(to, TYPE_STACK) || !IS_TYPE(from, TYPE_STACK))) {
        return 0;
    }
    if(from == to || count == 0) {
        return 0;
    }

    card cards[MAX_CARD_STACK];
    switch (TYPE(from)) {
    case TYPE_ACE: {
        AceStack *aces = &board->aces[INDEX(board->hovered)];
        if(aces->count == 0) {
            return 0;
        }
        cards[0] = aces->suit * 13 + aces->count - 1;
        break;
    }

    case TYPE_DRAW: {
        DrawStack *stack = &board->draw;
        if(stack->revealed == 0) {
            return 0;
        }
        cards[0] = stack->pile[stack->revealed];
        break;
    }

    case TYPE_STACK: {
        CardStack *stack = GET_STACK(board, from);
        if(count > stack->card_count) {
            return 0;
        }
        uint8_t offset = stack->card_count - count;
        for(uint8_t i = 0; i < count; i++) {
            cards[i] = stack->cards[i + offset];
        }
        break;
    }

    default:
        return 0;
    }
    
    switch (TYPE(to)) {
    case TYPE_ACE: board->aces[INDEX(to)].count++; break;
    case TYPE_DRAW: board->draw.revealed++; break; // TODO: make better
    case TYPE_STACK: {
        CardStack *stack = GET_STACK(board, to);
        if(count > MAX_CARD_STACK - stack->card_count) {
            return 0;
        }
        for(uint8_t i = 0; i < count; i++) {
            stack->cards[stack->card_count + i] = cards[i];
        }
        stack->card_count += count;
        break;
    }

    default:
        return 0;
    }

    switch (TYPE(from)) {
    case TYPE_ACE: board->aces[INDEX(from)].count--; break;
    case TYPE_DRAW: board->draw.revealed--; break;
    case TYPE_STACK: GET_STACK(board, from)->card_count -= count; break;
    }
    return 1;
}

void pickUp(Board *board) {
    if(IS_SELECTED(board->hovered, 1, TYPE_DRAW)) {
        flip(&board->draw);
        return;
    }
    board->hand_off_x = board->hovered_x - board->mouse_wspc_x;
    board->hand_off_y = board->hovered_y - board->mouse_wspc_y;
    switch (TYPE(board->hovered)) {
    case TYPE_STACK: {
        CardStack *cards = &board->stacks[INDEX(board->hovered)];
        cards->moving = 0;
        if(cards->card_count > board->stack_row) {
            moveCards(cards, &board->hand, cards->card_count - board->stack_row - 1);
        }
        break;
    }
    case TYPE_ACE: {
        AceStack *aces = &board->aces[INDEX(board->hovered)];
        aces->progression = 255;
        if(aces->count > 0) {
            board->hand.card_count = 1;
            board->hand.cards[0] = aces->suit * 13 + aces->count - 1;
            aces->count--;
        }
        break;
    }
    case TYPE_DRAW: {
        board->draw.pick = 0;
        board->draw.flip = 255;
        board->draw.progression = 255;

        board->hand.card_count = 1;
        board->hand.cards[0] = pick(&board->draw);
    }
    default:
        break;
    }
    board->return_to = board->hovered;
}

void returnHand(Board *board) {
    switch (TYPE(board->return_to)) {
    case TYPE_STACK: 
        if(board->hand.card_count > 0) {
            CardStack *cards = &board->stacks[INDEX(board->return_to)];
            CardAnim *card_anim = &board->stack_anims[INDEX(board->return_to)];
            cards->moving = board->hand.card_count;
            cards->progression = 0;
            card_anim->start_x = board->mouse_wspc_x + board->hand_off_x;
            card_anim->start_y = board->mouse_wspc_y + board->hand_off_y;
            moveCards(&board->hand, cards, 0);
            break;
        }
    
    case TYPE_ACE:
        if(board->hand.card_count == 1) {
            AceStack *ace = &board->aces[INDEX(board->return_to)];
            CardAnim *card_anim = &board->ace_anims[INDEX(board->return_to)];
            ace->progression = 0;
            card_anim->start_x = board->mouse_wspc_x + board->hand_off_x;
            card_anim->start_y = board->mouse_wspc_y + board->hand_off_y;
            ace->count++;
            board->hand.card_count = 0;
        }
        break;   

    case TYPE_DRAW:
        if(board->hand.card_count == 1) {
            unpick(&board->draw);
            board->hand.card_count = 0;
            board->draw.progression = 0;
            board->draw_anim.start_x = board->mouse_wspc_x + board->hand_off_x;
            board->draw_anim.start_y = board->mouse_wspc_y + board->hand_off_y;
        }
        break;

    default:
        break;
    }

    if(IS_TYPE(board->return_to, TYPE_STACK) && board->stacks[INDEX(board->return_to)].card_count == 0) {
        exposeCard(&board->stacks[INDEX(board->return_to)], board);
    }

    board->return_to = UNSELECTED;
}

void putDown(Board *board) {
    CardStack *hand = &board->hand;
    card top = hand->cards[0];
    uint8_t suit = top / 13, value = top % 13;
    uint8_t colour = suit / 2;

    switch (TYPE(board->hovered)) {
    case TYPE_ACE: {
        AceStack *aces = &board->aces[INDEX(board->hovered)];
        if(aces->count == 0) {
            aces->suit = suit;
        }
        if(hand->card_count == 1 && value == aces->count && aces->suit == suit) {
            aces->count++;
            hand->card_count = 0;
            CardAnim *card_anim = &board->ace_anims[INDEX(board->hovered)];
            aces->progression = 0;
            card_anim->start_x = board->mouse_wspc_x + board->hand_off_x;
            card_anim->start_y = board->mouse_wspc_y + board->hand_off_y;
        }
        break;
    }
    case TYPE_STACK: {
        CardStack *cards = &board->stacks[INDEX(board->hovered)];
        CardAnim *card_anim = &board->stack_anims[INDEX(board->hovered)];
        uint8_t allowed;
        if(cards->card_count > 0) {
            card bottom = cards->cards[cards->card_count - 1];
            allowed = colour != bottom / 26 && value == (bottom % 13) - 1;
        } else {
            allowed = value == 12;
        }
        if(allowed) {
            cards->moving = hand->card_count;
            cards->progression = 0;
            card_anim->start_x = board->mouse_wspc_x + board->hand_off_x;
            card_anim->start_y = board->mouse_wspc_y + board->hand_off_y;
            moveCards(hand, cards, 0);
        }
        break;
    }
    default:
        break;
    }
    returnHand(board);
}

void checkHovered(Board *board) {
    board->hovered = UNSELECTED;
    board->hovered_x = 0;
    board->hovered_y = 0;

    float x = board->mouse_wspc_x;
    float y = board->mouse_wspc_y;
    if(board->hand.card_count > 0) {
        x += board->hand_off_x + (CARD_W / 2);
        y += board->hand_off_y + (CARD_H / 2) + (STACK_OFF * (board->hand.card_count - 1));
    }
    
    CardStack *cards = board->stacks;
    for(int s = 0; s < STACKS; s++, cards++) {
        int stack_size = cards->card_count + cards->upside_down;
        float stack_x = STACK_X(s), stack_y = STACK_Y(stack_size);
        if(IN_BOX(x, y, stack_x, stack_y, STACK_W, STACK_H(stack_size))) {
            board->hovered = s | TYPE_STACK;
            board->stack_row = (y - STACK_Y(stack_size)) / STACK_OFF;
            if(board->stack_row < ROWS_PER_CARD) {
                board->stack_row = ROWS_PER_CARD;
            }
            board->stack_row -= ROWS_PER_CARD;
            board->hovered_x = stack_x;
            board->hovered_y = stack_y;
            return;
        }
    }
    AceStack *aces = board->aces;
    for(int a = 0; a < ACES; a++, aces++) {
        float ace_x = ACE_X(a), ace_y = ACE_Y;
        if(IN_BOX(x, y, ace_x, ace_y, ACE_W, ACE_H)) {
            board->hovered = a | TYPE_ACE;
            board->hovered_x = ace_x;
            board->hovered_y = ace_y;
            return;
        }
    }
    float flip_x = FLIP_X, flip_y = FLIP_Y;
    if(IN_BOX(x, y, flip_x, flip_y, FLIP_W, FLIP_H)) {
        board->hovered = 1 | TYPE_DRAW;
        return;
    }

    if(board->draw.revealed > 0) {
        float draw_x = DRAW_X(0, board->draw.revealed), draw_y = DRAW_Y;
        if(IN_BOX(x, y, draw_x, draw_y, DRAW_W, DRAW_H)) {
            board->hovered = TYPE_DRAW;
            board->hovered_x = draw_x;
            board->hovered_y = draw_y;
            return;
        }
    }
}

void resetStack(CardStack *stack, uint8_t upside_down) {
    stack->upside_down = upside_down;
    stack->card_count = 0;
    stack->moving = 0;
    stack->progression = 255;
    stack->flip = 255;
}

void resetBoard(Board *board) {
    srand(time(NULL));
    for(int c = 0; c < CARD_TYPES; c++) {
        board->order[c] = c;
    }
    board->cards_left = CARD_TYPES;
    for(int s = 0; s < STACKS; s++) {
        resetStack(&board->stacks[s], s);
        newCard(&board->stacks[s], board);
    }
    for(int a = 0; a < ACES; a++) {
        board->aces[a].count = 0;
        board->aces[a].progression = 255;
    }
    resetStack(&board->hand, 0);
    board->highlighted = UNSELECTED;
    board->return_to = UNSELECTED;
    board->hovered = UNSELECTED;

    board->draw.from = 0;
    board->draw.revealed = 0;
    board->draw.progression = 255;
    board->draw.pick = 255;
    board->draw.flip = 255;
    board->draw.size = DRAW_CARDS;
    for(int d = 0; d < DRAW_CARDS; d++) {
        board->draw.pile[d] = getCard(board);
    }
}