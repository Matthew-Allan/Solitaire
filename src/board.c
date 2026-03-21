#include "board.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TOP_MARGIN (DISPLAY_HEIGHT - CARD_H - MARGIN)

#define ROWS_PER_CARD (1.f / STACK_OFF - 1)

#define COL_X(row) (MARGIN + COL_OFF * (row))

#define ACE_X(ace) COL_X(STACKS - ACES + ace)
#define ACE_Y TOP_MARGIN
#define ACE_W CARD_W
#define ACE_H CARD_H

#define STACK_X(stack) COL_X(stack)
#define STACK_Y(size) ((ACE_Y - ACE_H - MARGIN) - STACK_OFF * ((size) - 1))
#define STACK_W CARD_W
#define STACK_H(size) (CARD_H + STACK_OFF * ((size) - 1))

#define FLIP_X COL_X(0)
#define FLIP_Y TOP_MARGIN
#define FLIP_W CARD_W
#define FLIP_H CARD_H

#define DRAW_X(card, revealed) (COL_X(1) + DRAW_OFF * (((revealed) > DRAW_SHOWN ? DRAW_SHOWN : (revealed)) - (card) - 1))
#define DRAW_Y TOP_MARGIN
#define DRAW_W CARD_W
#define DRAW_H CARD_H

#define IN_BOX(x, y, b_x, b_y, b_w, b_h) (x >= (b_x) && x < (b_x) + (b_w) && y >= (b_y) && y < (b_y) + (b_h))

int pick(DrawStack *stack) {
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
    if(stack->from == stack->size) {
        stack->size = stack->revealed;
        stack->revealed = 0;
        stack->from = 0;
        return;
    }

    for(int i = 0; i < 3 && stack->from < stack->size; i++, stack->from++, stack->revealed++) {
        stack->pile[stack->revealed] = stack->pile[stack->from];
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

void addCardData(Card *cards, uint8_t *count, int card, float x, float y) {
    if(*count >= MAX_CARDS) {
        *count = MAX_CARDS;
        return;
    }
    Card *card_pos = cards + *count;
    card_pos->card = card;
    card_pos->y = y + (CARD_H / 2);
    card_pos->x = x + (CARD_W / 2);
    (*count)++;
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
        board->hand.card_count = 1;
        board->hand.cards[0] = pick(&board->draw);
        board->draw.progression = 255;
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

#define EXP_DIV(prog) (1 + expf(-0.04 * ((float)(prog) - 127.5)))

void animateSingle(uint8_t p, float s_x, float e_x, float *x) {
    *x = (e_x - s_x) / EXP_DIV(p) + s_x;
}

void animate(uint8_t p, CardAnim *anim, float e_x, float e_y, float *x, float *y) {
    float div = EXP_DIV(p);
    *x = (e_x - anim->start_x) / div + anim->start_x;
    *y = (e_y - anim->start_y) / div + anim->start_y;
}

void addAceAnim(Board *board, uint8_t stack, Card *cards, uint8_t *count) {
    AceStack *ace = &board->aces[stack];
    if(ace->progression == 255) {
        return;
    }
    CardAnim *anim = &board->ace_anims[stack];
    float x, y;
    animate(ace->progression, anim, ACE_X(stack), ACE_Y, &x, &y);
    
    addCardData(cards, count, ace->count - 1 + (ace->suit * 13), x, y);
}

void addStackAnim(Board *board, uint8_t stack, Card *cards, uint8_t *count) {
    CardStack *card_stack = &board->stacks[stack];
    if(card_stack->moving == 0) {
        return;
    }
    CardAnim *anim = &board->stack_anims[stack];
    float x, y;
    animate(card_stack->progression, anim, STACK_X(stack), STACK_Y(card_stack->card_count + card_stack->upside_down), &x, &y);

    for(int c = card_stack->card_count - 1; c >= card_stack->card_count - card_stack->moving; c--, y += STACK_OFF) {
        addCardData(cards, count, card_stack->cards[c], x, y);
    }
}

void addStack(Board *board, CardStack *stack, float x, float y, Card *cards, uint8_t *count) {
    for(int c = stack->card_count - stack->moving - 1; c >= 0; c--, y += STACK_OFF) {
        addCardData(cards, count, FLIP_CARD(stack->cards[c], stack->flip), x, y);
    }
    for(int b = 0; b < stack->upside_down; b++, y += STACK_OFF) {
        addCardData(cards, count, FLIPPED_CARD, x, y);
    }
}

void updateProgression(uint8_t *prog, uint8_t delta) {
    *prog = delta > 255 - *prog ? 255 : *prog + delta;
}

void updateAnims(Board *board) {
    for(int s = 0; s < STACKS; s++) {
        updateProgression(&board->stacks[s].progression, ANIM_SPEED);
        updateProgression(&board->stacks[s].flip, ANIM_SPEED);
        if(board->stacks[s].progression == 255) {
            board->stacks[s].moving = 0;
        }
    }
    for(int a = 0; a < ACES; a++) {
        updateProgression(&board->aces[a].progression, ANIM_SPEED);
    }
    updateProgression(&board->draw.progression, ANIM_SPEED);
    updateProgression(&board->draw.flip, ANIM_SPEED);
}

void updateCards(Board *board, Card *cards, uint8_t *count) {
    *count = 0;
    board->highlighted = -1;
    float hand_x = board->mouse_wspc_x + board->hand_off_x;
    float hand_y = board->mouse_wspc_y + board->hand_off_y;
    DrawStack *draw = &board->draw;
    uint8_t draw_from = draw->revealed;
    addStack(board, &board->hand, hand_x, hand_y, cards, count);

    for(int s = 0; s < STACKS; s++) {
        addStackAnim(board, s, cards, count);
    }
    for(int a = 0; a < ACES; a++) {
        addAceAnim(board, a, cards, count);
    }
    if(draw->progression < 255) {
        float x, y;
        animate(draw->progression, &board->draw_anim, DRAW_X(0, draw->revealed), DRAW_Y, &x, &y);
        addCardData(cards, count, draw->pile[draw_from - 1], x, y);
        draw_from--;
    }

    for(int s = 0; s < STACKS; s++) {
        CardStack *stack = &board->stacks[s];
        if(IS_SELECTED(board->hovered, s, TYPE_STACK) && stack->card_count > 0 && stack->moving <= board->stack_row) {
            board->highlighted = *count + board->stack_row;
        }
        float x = STACK_X(s);
        float y = STACK_Y(stack->card_count + stack->upside_down - stack->moving);
        addStack(board, stack, x, y, cards, count);
    }
    for(int a = 0; a < ACES; a++) {
        AceStack *stack = &board->aces[a];
        if(IS_SELECTED(board->hovered, a, TYPE_ACE)) {
            board->highlighted = *count;
        }
        uint8_t top = stack->count - (stack->progression < 255);
        if(top == 0) {
            addCardData(cards, count, FLIPPED_CARD, ACE_X(a), ACE_Y);
        } else {
            addCardData(cards, count, top - 1 + stack->suit * 13, ACE_X(a), ACE_Y);
        }
    }

    if(IS_SELECTED(board->hovered, 0, TYPE_DRAW)) {
        board->highlighted = *count;
    }

    int d = draw->progression < 255;
    if(d && draw_from >= DRAW_SHOWN) {
        for(; d < DRAW_SHOWN; d++, draw_from--) {
            float s_x = DRAW_X(d - 1, draw->revealed), e_x = DRAW_X(d, draw->revealed), x;
            animateSingle(draw->progression, s_x, e_x, &x);
            addCardData(cards, count, draw->pile[draw_from - 1], x, DRAW_Y);
        }
        addCardData(cards, count, draw->pile[draw_from - 1], DRAW_X(d - 1, draw->revealed), DRAW_Y);
    } else {
        for(; draw_from > 0 && d < DRAW_SHOWN; d++, draw_from--) {
            float s_x = FLIP_X, e_x = DRAW_X(d, draw->revealed), x;
            animateSingle(draw->flip, s_x, e_x, &x);
            addCardData(cards, count, FLIP_CARD(draw->pile[draw_from - 1], draw->flip), x, DRAW_Y);
        }
    }
    if(draw->from != draw->size) {
        addCardData(cards, count, FLIPPED_CARD, FLIP_X, FLIP_Y);
    }
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
    board->draw.flip = 255;
    board->draw.size = DRAW_CARDS;
    for(int d = 0; d < DRAW_CARDS; d++) {
        board->draw.pile[d] = getCard(board);
    }
}