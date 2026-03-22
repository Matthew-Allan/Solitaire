#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define MAX_CARDS 128
#define MAX_CARD_STACK 13

#define STACKS 7
#define ACES 4
#define DRAW_SHOWN 3

#define CARD_TYPES 52

#define DISPLAY_HEIGHT 4

#define MARGIN 0.125
#define COL_OFF 0.6875
#define STACK_OFF 0.125
#define DRAW_OFF 0.25

#define CARD_PIXEL_W 62
#define CARD_PIXEL_H 98

#define ANIM_SPEED 16

#define STACK_CARDS ((STACKS * (STACKS + 1)) / 2)
#define DRAW_CARDS (CARD_TYPES - STACK_CARDS)

#define CARD_W ((float) CARD_PIXEL_W / CARD_PIXEL_H)
#define CARD_H 1.f

#define TYPE_NONE  0x00
#define TYPE_STACK 0x40
#define TYPE_ACE   0x80
#define TYPE_DRAW  0xC0

#define UNSELECTED TYPE_NONE

#define TYPE(field) ((field) & 0xC0)
#define INDEX(field) ((field) & 0x3F)

#define FLIP_CARD(card, prog) (((255 - (uint8_t)(prog)) << 8) | (uint8_t)(card))
#define FLIPPED_CARD FLIP_CARD(0, 0)

#define IS_TYPE(field, type) (TYPE(field) == (type))
#define IS_INDEX(field, index) (INDEX(field) == (index))
#define IS_SELECTED(field, index, type) (((index) | (type)) == (field))

typedef uint8_t card;
typedef uint8_t cardloc;

typedef struct Card {
    float x;
    float y;
    int card;
} Card;

typedef struct CardStack {
    uint8_t upside_down;
    uint8_t card_count;
    uint8_t moving;
    uint8_t progression;
    uint8_t flip;
    card cards[MAX_CARD_STACK];
} CardStack;

typedef struct AceStack {
    uint8_t suit;
    uint8_t count;
    uint8_t progression;
} AceStack;

typedef struct DrawStack {
    uint8_t from;
    uint8_t revealed;
    uint8_t size;
    uint8_t flip;
    uint8_t recent_flip;
    uint8_t progression;
    card pile[DRAW_CARDS];
} DrawStack;

typedef struct CardAnim {
    float start_x;
    float start_y;
} CardAnim;

typedef struct Board {
    float hand_off_x;
    float hand_off_y;
    float mouse_wspc_x;
    float mouse_wspc_y;
    float hovered_x;
    float hovered_y;
    CardAnim draw_anim;
    CardAnim stack_anims[STACKS];
    CardAnim ace_anims[ACES];
    CardStack stacks[STACKS];
    AceStack aces[ACES];
    CardStack hand;
    DrawStack draw;
    cardloc return_to;
    cardloc hovered;
    uint8_t stack_row;
    uint8_t highlighted;
    uint8_t cards_left;
    card order[CARD_TYPES];
} Board;

void resetBoard(Board *board);

void checkHovered(Board *board);
void updateCards(Board *board, Card *cards, uint8_t *count);
void updateAnims(Board *board);

void pickUp(Board *board);
void putDown(Board *board);

#endif