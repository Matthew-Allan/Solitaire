#include "layout.h"
#include <math.h>

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
    updateProgression(&board->draw.pick, ANIM_SPEED);
    updateProgression(&board->draw.flip, ANIM_SPEED);
    if(board->draw.flip == 255) {
        board->draw.recent_flip = 0;
    }
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

void updateDrawCards(Board *board, Card *cards, uint8_t *count) {
    DrawStack *draw = &board->draw;
    uint8_t draw_from = draw->revealed - (draw->progression < 255);
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
    } else if(draw->revealed == 0 && draw->flip < 255) {
        uint8_t end = draw->size;
        for(; end > 0 && d < DRAW_SHOWN; d++, end--) {
            float s_x = DRAW_X(d, draw->size), e_x = FLIP_X, x;
            animateSingle(draw->flip, s_x, e_x, &x);
            addCardData(cards, count, FLIP_CARD(draw->pile[end - 1], 255 - draw->flip), x, DRAW_Y);
        }
    } else if(draw->pick < 255 && draw_from >= DRAW_SHOWN) {
        for(; draw_from > 0 && d < DRAW_SHOWN; d++, draw_from--) {
            float s_x = DRAW_X(d + 1 < DRAW_SHOWN ? d + 1 : d, draw->revealed), e_x = DRAW_X(d, draw->revealed), x;
            animateSingle(draw->pick, s_x, e_x, &x);
            addCardData(cards, count, draw->pile[draw_from - 1], x, DRAW_Y);
        }
    } else if(draw->flip < 255) {
        for(; draw_from > 0 && d < draw->recent_flip; d++, draw_from--) {
            float s_x = FLIP_X, e_x = DRAW_X(d, draw->revealed), x;
            animateSingle(draw->flip, s_x, e_x, &x);
            addCardData(cards, count, FLIP_CARD(draw->pile[draw_from - 1], draw->flip), x, DRAW_Y);
        }
        for(; draw_from > 0 && d < DRAW_SHOWN + draw->recent_flip; d++, draw_from--) {
            float s_x = DRAW_X(d - draw->recent_flip, draw->revealed - draw->recent_flip), e_x = d >= DRAW_SHOWN ? DRAW_X(0, 1) : DRAW_X(d, draw->revealed), x;
            animateSingle(draw->flip, s_x, e_x, &x);
            addCardData(cards, count, draw->pile[draw_from - 1], x, DRAW_Y);
        }
    } else {
        for(; draw_from > 0 && d < DRAW_SHOWN; d++, draw_from--) {
            addCardData(cards, count, draw->pile[draw_from - 1], DRAW_X(d, draw->revealed), DRAW_Y);
        }
    }
    if(draw->from != draw->size && !(draw->revealed == 0 && draw->flip < 255)) {
        addCardData(cards, count, FLIPPED_CARD, FLIP_X, FLIP_Y);
    }
}

void updateAceCards(Board *board, Card *cards, uint8_t *count) {
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
}

void addStack(Board *board, CardStack *stack, float x, float y, Card *cards, uint8_t *count) {
    for(int c = stack->card_count - stack->moving - 1; c >= 0; c--, y += STACK_OFF) {
        addCardData(cards, count, FLIP_CARD(stack->cards[c], stack->flip), x, y);
    }
    for(int b = 0; b < stack->upside_down; b++, y += STACK_OFF) {
        addCardData(cards, count, FLIPPED_CARD, x, y);
    }
}

void updateStackCards(Board *board, Card *cards, uint8_t *count) {
    for(int s = 0; s < STACKS; s++) {
        CardStack *stack = &board->stacks[s];
        if(IS_SELECTED(board->hovered, s, TYPE_STACK) && stack->card_count > 0 && stack->moving <= board->stack_row) {
            board->highlighted = *count + board->stack_row;
        }
        float x = STACK_X(s);
        float y = STACK_Y(stack->card_count + stack->upside_down - stack->moving);
        addStack(board, stack, x, y, cards, count);
    }
}

void addAceAnim(Board *board, Card *cards, uint8_t *count) {
    for(int stack = 0; stack < ACES; stack++) {
        AceStack *ace = &board->aces[stack];
        if(ace->progression == 255) {
            continue;;
        }
        CardAnim *anim = &board->ace_anims[stack];
        float x, y;
        animate(ace->progression, anim, ACE_X(stack), ACE_Y, &x, &y);
        
        addCardData(cards, count, ace->count - 1 + (ace->suit * 13), x, y);
    }
}

void addStackAnim(Board *board, Card *cards, uint8_t *count) {
    for(int stack = 0; stack < STACKS; stack++) {
        CardStack *card_stack = &board->stacks[stack];
        if(card_stack->moving == 0) {
            continue;
        }
        CardAnim *anim = &board->stack_anims[stack];
        float x, y;
        animate(card_stack->progression, anim, STACK_X(stack), STACK_Y(card_stack->card_count + card_stack->upside_down), &x, &y);

        for(int c = card_stack->card_count - 1; c >= card_stack->card_count - card_stack->moving; c--, y += STACK_OFF) {
            addCardData(cards, count, card_stack->cards[c], x, y);
        }
    }
}

void addDrawAnim(Board *board, Card *cards, uint8_t *count) {
    DrawStack *draw = &board->draw;
    if(draw->progression == 255) {
        return;
    }
    float x, y;
    animate(draw->progression, &board->draw_anim, DRAW_X(0, draw->revealed), DRAW_Y, &x, &y);
    addCardData(cards, count, draw->pile[draw->revealed - 1], x, y);
}

void updateCards(Board *board, Card *cards, uint8_t *count) {
    *count = 0;
    board->highlighted = -1;
    float hand_x = board->mouse_wspc_x + board->hand_off_x;
    float hand_y = board->mouse_wspc_y + board->hand_off_y;
    addStack(board, &board->hand, hand_x, hand_y, cards, count);

    addDrawAnim(board, cards, count);
    addStackAnim(board, cards, count);
    addAceAnim(board, cards, count);
    updateAceCards(board, cards, count);
    updateStackCards(board, cards, count);
    updateDrawCards(board, cards, count);
}