#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>
#include <seng/quaternion.h>

#include <stdlib.h>
#include <time.h>

#include "cardshader.h"

#define MAX_CARDS 128
#define MAX_CARD_STACK 13

#define CARD_TYPES 52

#define DISPLAY_HEIGHT 4

#define CARD_PIXEL_W 62
#define CARD_PIXEL_H 98

#define STACKS 7
#define ACES 4

#define STACK_CARDS ((STACKS * (STACKS + 1)) / 2)
#define DRAW_CARDS (CARD_TYPES - STACK_CARDS)

#define CARD_W ((float) CARD_PIXEL_W / CARD_PIXEL_H)
#define CARD_H 1

#define MARGIN 0.125
#define COL_OFF 0.6875
#define STACK_OFF 0.125
#define DRAW_OFF 0.25

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

#define DRAW_SHOWN 3

#define DRAW_X(card, revealed) (COL_X(1) + DRAW_OFF * (((revealed) > DRAW_SHOWN ? DRAW_SHOWN : (revealed)) - (card) - 1))
#define DRAW_Y TOP_MARGIN
#define DRAW_W CARD_W
#define DRAW_H CARD_H

#define IN_BOX(x, y, b_x, b_y, b_w, b_h) (x >= (b_x) && x < (b_x) + (b_w) && y >= (b_y) && y < (b_y) + (b_h))

typedef struct Card {
    float x;
    float y;
    int card;
} Card;

typedef struct CardStack {
    uint8_t upside_down;
    uint8_t card_count;
    uint8_t cards[MAX_CARD_STACK];
} CardStack;

typedef struct AceStack {
    uint8_t suit;
    uint8_t count;
} AceStack;
typedef struct DrawStack {
    uint8_t from;
    uint8_t revealed;
    uint8_t size;
    uint8_t pile[DRAW_CARDS];
} DrawStack;

typedef struct Board {
    float hand_off_x;
    float hand_off_y;
    AceStack aces[ACES];
    CardStack stacks[STACKS];
    CardStack hand;
    DrawStack draw;
    uint8_t active_draw;
    uint8_t return_to;
    uint8_t hovered;
    uint8_t stack_row;
    uint8_t highlighted;
    uint8_t cards_left;
    uint8_t order[CARD_TYPES];
} Board;

uint8_t pick(DrawStack *stack) {
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

#define TYPE_NONE  0x00
#define TYPE_STACK 0x40
#define TYPE_ACE   0x80
#define TYPE_DRAW  0xC0

#define UNSELECTED TYPE_NONE

#define TYPE(field) ((field) & 0xC0)
#define INDEX(field) ((field) & 0x3F)

#define IS_TYPE(field, type) (TYPE(field) == (type))
#define IS_INDEX(field, index) (INDEX(field) == (index))
#define IS_SELECTED(field, index, type) (((index) | (type)) == (field))

int getCard(Board *board) {
    if(board->cards_left == 0) {
        return -1;
    }
    uint8_t choice = rand() % board->cards_left;
    board->cards_left--;

    uint8_t card = board->order[choice];
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

void resetBoard(Board *board) {
    for(int c = 0; c < CARD_TYPES; c++) {
        board->order[c] = c;
    }
    board->cards_left = CARD_TYPES;
    for(int s = 0; s < STACKS; s++) {
        board->stacks[s].upside_down = s;
        board->stacks[s].card_count = 0;
        newCard(&board->stacks[s], board);
    }
    for(int a = 0; a < ACES; a++) {
        board->aces[a].count = 0;
    }
    board->hand.card_count = 0;
    board->hand.upside_down = 0;
    board->highlighted = UNSELECTED;
    board->return_to = UNSELECTED;
    board->hovered = UNSELECTED;

    board->draw.from = 0;
    board->draw.revealed = 0;
    board->draw.size = DRAW_CARDS;
    for(int d = 0; d < DRAW_CARDS; d++) {
        board->draw.pile[d] = getCard(board);
    }
}

void addCard(Card *cards, uint8_t *count, int card, float x, float y) {
    if(*count >= MAX_CARDS) {
        *count = MAX_CARDS;
        return;
    }
    Card *card_pos = cards + *count;
    card_pos->card = card;
    card_pos->y = y;
    card_pos->x = x;
    (*count)++;
}

void addStack(Board *board, CardStack *stack, float x, float y, Card *cards, uint8_t *count) {
    for(int c = stack->card_count - 1; c >= 0; c--, y += STACK_OFF) {
        addCard(cards, count, stack->cards[c], x, y);
    }
    for(int b = 0; b < stack->upside_down; b++, y += STACK_OFF) {
        addCard(cards, count, 64, x, y);
    }
}

void updateCards(Board *board, float mouse_x, float mouse_y, Card *cards, uint8_t *count) {
    *count = 0;
    board->highlighted = -1;
    addStack(board, &board->hand, mouse_x, mouse_y, cards, count);
    for(int s = 0; s < STACKS; s++) {
        CardStack *stack = &board->stacks[s];
        if(IS_SELECTED(board->hovered, s, TYPE_STACK) && stack->card_count > 0) {
            board->highlighted = *count + board->stack_row;
        }
        float x = STACK_X(s);
        float y = STACK_Y(stack->card_count + stack->upside_down);
        addStack(board, stack, x, y, cards, count);
    }
    for(int a = 0; a < ACES; a++) {
        AceStack *stack = &board->aces[a];
        if(IS_SELECTED(board->hovered, a, TYPE_ACE)) {
            board->highlighted = *count;
        }
        if(stack->count == 0) {
            addCard(cards, count, 64, ACE_X(a), ACE_Y);
        } else {
            addCard(cards, count, stack->count - 1 + stack->suit * 13, ACE_X(a), ACE_Y);
        }
    }
    DrawStack *draw = &board->draw;
    if(draw->from != draw->size) {
        addCard(cards, count, 64, FLIP_X, FLIP_Y);
    }

    uint8_t draw_from = draw->revealed;
    if(IS_SELECTED(board->hovered, 0, TYPE_DRAW)) {
        board->highlighted = *count;
    }
    for(int d = 0; draw_from > 0 && d < DRAW_SHOWN; d++, draw_from--) {
        addCard(cards, count, draw->pile[draw_from - 1], DRAW_X(d, draw->revealed), DRAW_Y);
    }
}

void checkHovered(Board *board, float x, float y) {
    board->hovered = UNSELECTED;
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
            return;
        }
    }
    AceStack *aces = board->aces;
    for(int a = 0; a < ACES; a++, aces++) {
        float ace_x = ACE_X(a), ace_y = ACE_Y;
        if(IN_BOX(x, y, ace_x, ace_y, ACE_W, ACE_H)) {
            board->hovered = a | TYPE_ACE;
            return;
        }
    }
    float flip_x = FLIP_X, flip_y = FLIP_Y;
    if(IN_BOX(x, y, flip_x, flip_y, FLIP_W, FLIP_H)) {
        board->hovered = DRAW_SHOWN | TYPE_DRAW;
        return;
    }

    if(board->draw.revealed > 0) {
        float draw_x = DRAW_X(0, board->draw.revealed), draw_y = DRAW_Y;
        if(IN_BOX(x, y, draw_x, draw_y, DRAW_W, DRAW_H)) {
            board->hovered = TYPE_DRAW;
        }
    }
}

void pickUp(Board *board) {
    switch (TYPE(board->hovered)) {
    case TYPE_STACK: {
        CardStack *cards = &board->stacks[INDEX(board->hovered)];
        if(cards->card_count > board->stack_row) {
            moveCards(cards, &board->hand, cards->card_count - board->stack_row - 1);
        }
        break;
    }
    case TYPE_ACE: {
        AceStack *aces = &board->aces[INDEX(board->hovered)];
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
    }
    default:
        break;
    }
    board->return_to = board->hovered;
}

void returnHand(Board *board) {
    switch (TYPE(board->return_to)) {
    case TYPE_STACK: {
        CardStack *cards = &board->stacks[INDEX(board->return_to)];
        moveCards(&board->hand, cards, 0);
        if(cards->card_count == 0) {
            exposeCard(cards, board);
        }
        break;
    }
    
    case TYPE_ACE:
        if(board->hand.card_count == 1) {
            board->aces[INDEX(board->return_to)].count++;
            board->hand.card_count = 0;
        }
        break;
    
    case TYPE_DRAW:
        if(board->hand.card_count == 1) {
            unpick(&board->draw);
            board->hand.card_count = 0;
        }
        break;
    default:
        break;
    }

    board->return_to = UNSELECTED;
}

void putDown(Board *board) {
    CardStack *hand = &board->hand;
    uint8_t top = hand->cards[0];
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
        }
        break;
    }
    case TYPE_STACK: {
        CardStack *cards = &board->stacks[INDEX(board->hovered)];
        uint8_t allowed;
        if(cards->card_count > 0) {
            uint8_t bottom = cards->cards[cards->card_count - 1];
            allowed = colour != bottom / 26 && value == (bottom % 13) - 1;
        } else {
            allowed = value == 12;
        }
        if(allowed) {
            moveCards(hand, cards, 0);
        }
        break;
    }
    default:
        break;
    }
    returnHand(board);
}

int pollEvents(Program *program, Board *board, Camera *cam) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            program->running = 0;
            break;
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                updateWinDims(program);
                updateOrtho(cam, program->aspect * DISPLAY_HEIGHT, DISPLAY_HEIGHT);
            }
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.scancode == SDL_SCANCODE_P) {
                toggleFullscreen(program);
            } else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                resetBoard(board);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_LEFT) {
                if(IS_SELECTED(board->hovered, DRAW_SHOWN, TYPE_DRAW)) {
                    flip(&board->draw);
                } else {
                    pickUp(board);
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT) {
                putDown(board);
            }
            break;
        }
    }
    return 0;
}

int cardVAO(VertexArrObj *card_vao) {
    boxVAO_2D(card_vao, CARD_W, CARD_H, 0, 0, CARD_PIXEL_H, CARD_PIXEL_W);
    glGenBuffers(1, &card_vao->vbo);
    glBindVertexArray(card_vao->id);

    glBindBuffer(GL_ARRAY_BUFFER, card_vao->vbo);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Card), (void*)0);
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Card), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(3, 1);
    return 0;
}

// Run the game and gameloop in the given window.
int runGame(Program *program) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ObjectShader obj_shad;
    if(buildObjectShader(&obj_shad) < 0) {
        return -1;
    }

    VertexArrObj card_vao;
    cardVAO(&card_vao);

    GLuint atlas; 
    if(loadTex("textures/atlas.png", &atlas, 0, GL_RGBA8UI, GL_RGBA_INTEGER) < 0) {
        return -1;
    }
    glUseProgram(obj_shad.program);
    glUniform1i(obj_shad.atlas, 0);

    Camera cam;
    vec3 pos = vec3(0, 0, 0);
    initOrthoCam(&cam, pos, program->aspect * DISPLAY_HEIGHT, DISPLAY_HEIGHT);
    quatFromEuler(cam.trans.orientation, 0, 0, 0);

    Board board;
    resetBoard(&board);
        
    Card cards[MAX_CARDS];
    uint8_t count = 0;

    // Main game loop
    while(program->running) {
        pollEvents(program, &board, &cam);

        float x, y;
        getMouseNDC(program, &x, &y);
        screenToWorld(&cam, &x, &y);

        checkHovered(&board, x, y);
        updateCards(&board, x, y, cards, &count);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(obj_shad.program);

        uploadCamMat(&cam, obj_shad.camera);

        glUniform1i(obj_shad.selected, board.highlighted);
        updateVBO(&card_vao, cards, sizeof(cards));

        glBindTexture(GL_TEXTURE_2D, atlas);
        drawVAOInstanced(&card_vao, count);

        // Swap the buffers.
        SDL_GL_SwapWindow(program->window);
        waitForFrame(program);
    }

    return 0;
}

int main(int argc, char const *argv[]) {
    srand(time(NULL));

    // Create the window
    Program program;
    if(createProgram(&program) == 0) {
        runGame(&program);
    }

    // Quit and return.
    SDL_Quit();
    return 0;
}
