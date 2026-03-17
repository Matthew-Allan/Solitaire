#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>
#include <seng/quaternion.h>

#include <stdlib.h>
#include <time.h>

#include "objectshad.h"

#define MAX_CARDS 128
#define MAX_CARD_STACK 13

#define UNSELECTED 255

#define CARD_TYPES 52

#define DISPLAY_HEIGHT 4

#define CARD_PIXEL_W 62
#define CARD_PIXEL_H 98

#define CARD_W ((float) CARD_PIXEL_W / CARD_PIXEL_H)
#define CARD_H 1

#define MARGIN 0.125
#define TOP_MARGIN (DISPLAY_HEIGHT - CARD_H - MARGIN)

#define STACK_Y_OFF 0.125
#define STACK_X_OFF 0.6875

#define ROWS_PER_CARD (1.f / STACK_Y_OFF - 1)

#define ACE_OFF (STACK_X_OFF)

#define ACE_X(ace) (STACK_X(3) + ACE_OFF * (ace))
#define ACE_Y TOP_MARGIN

#define ACE_W CARD_W
#define ACE_H CARD_H

#define STACK_X(stack) (MARGIN + STACK_X_OFF * (stack))
#define STACK_Y(size) ((ACE_Y - ACE_H - MARGIN) - STACK_Y_OFF * ((size) - 1))

#define STACK_W CARD_W
#define STACK_H(size) (CARD_H + STACK_Y_OFF * ((size) - 1))

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

typedef struct Board {
    CardStack stacks[7];
    CardStack hand;
    float hand_off_x;
    float hand_off_y;
    AceStack aces[4];
    uint8_t return_hand;
    uint8_t selected_stack;
    uint8_t selected_row;
    uint8_t selected_ace;
    uint8_t selected_card;
    uint8_t cards_left;
    uint8_t order[CARD_TYPES];
} Board;


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
    for(int i = 0; i < CARD_TYPES; i++) {
        board->order[i] = i;
    }
    board->cards_left = CARD_TYPES;
    for(int i = 0; i < 7; i++) {
        board->stacks[i].upside_down = i;
        board->stacks[i].card_count = 0;
        newCard(&board->stacks[i], board);
    }
    for(int i = 0; i < 4; i++) {
        board->aces[i].count = 0;
    }
    board->hand.card_count = 0;
    board->hand.upside_down = 0;
    board->selected_card = UNSELECTED;
    board->return_hand = UNSELECTED;
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
    for(int c = stack->card_count - 1; c >= 0; c--, y += STACK_Y_OFF) {
        addCard(cards, count, stack->cards[c], x, y);
    }
    for(int b = 0; b < stack->upside_down; b++, y += STACK_Y_OFF) {
        addCard(cards, count, 64, x, y);
    }
}

void updateCards(Board *board, float mouse_x, float mouse_y, Card *cards, uint8_t *count) {
    *count = 0;
    if(board->selected_stack == UNSELECTED) {
        board->selected_card = UNSELECTED;
    }
    addStack(board, &board->hand, mouse_x, mouse_y, cards, count);
    for(int s = 0; s < 7; s++) {
        CardStack *stack = &board->stacks[s];
        if(board->selected_stack == s && stack->card_count > 0) {
            board->selected_card = *count + board->selected_row;
        }
        float x = STACK_X(s);
        float y = STACK_Y(stack->card_count + stack->upside_down);
        addStack(board, stack, x, y, cards, count);
    }
    for(int a = 0; a < 4; a++) {
        AceStack *stack = &board->aces[a];
        if(board->selected_ace == a) {
            board->selected_card = *count;
        }
        if(stack->count == 0) {
            addCard(cards, count, board->selected_ace == a ? 0 : 64, ACE_X(a), ACE_Y);
        } else {
            addCard(cards, count, stack->count - 1 + stack->suit * 13, ACE_X(a), ACE_Y);
        }
    }
}

void checkHovered(Board *board, float x, float y) {
    board->selected_row = UNSELECTED;
    board->selected_stack = UNSELECTED;
    board->selected_ace = UNSELECTED;
    CardStack *cards = board->stacks;
    for(int s = 0; s < 7; s++, cards++) {
        int stack_size = cards->card_count + cards->upside_down;
        float stack_x = STACK_X(s), stack_y = STACK_Y(stack_size);
        float stack_w = STACK_W,    stack_h = STACK_H(stack_size);
        if(IN_BOX(x, y, stack_x, stack_y, stack_w, stack_h)) {
            board->selected_stack = s;
            board->selected_row = (y - STACK_Y(stack_size)) / STACK_Y_OFF;
            if(board->selected_row < ROWS_PER_CARD) {
                board->selected_row = ROWS_PER_CARD;
            }
            board->selected_row -= ROWS_PER_CARD;
            return;
        }
    }
    AceStack *aces = board->aces;
    for(int a = 0; a < 4; a++, aces++) {
        float ace_x = ACE_X(a), ace_y = ACE_Y;
        float ace_w = ACE_W,    ace_h = ACE_H;
        if(IN_BOX(x, y, ace_x, ace_y, ace_w, ace_h)) {
            board->selected_ace = a;
            return;
        }
    }
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
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_LEFT) {
                if(board->selected_stack != UNSELECTED) {
                    CardStack *stack = &board->stacks[board->selected_stack];
                    if(stack->card_count > board->selected_row) {
                        moveCards(stack, &board->hand, stack->card_count - board->selected_row - 1);
                        board->return_hand = board->selected_stack;
                    }
                }
            } else if(event.button.button == SDL_BUTTON_RIGHT) {
                if(board->selected_stack != UNSELECTED) {
                    CardStack *stack = &board->stacks[board->selected_stack];
                    newCard(stack, board);
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT) {
                CardStack *hand = &board->hand;
                uint8_t top = hand->cards[0];
                uint8_t suit = top / 13;
                uint8_t colour = suit / 2;
                uint8_t value = top % 13;

                if(board->selected_ace != UNSELECTED) {
                    AceStack *stack = &board->aces[board->selected_ace];
                    if(stack->count == 0) {
                        stack->suit = suit;
                    }
                    if(hand->card_count == 1 && value == stack->count && stack->suit == suit) {
                        stack->count++;
                        hand->card_count = 0;
                    }
                }

                if(board->selected_stack != UNSELECTED) {
                    CardStack *new_stack = &board->stacks[board->selected_stack];
                    uint8_t bottom = new_stack->card_count > 0 ? new_stack->cards[new_stack->card_count - 1] : UNSELECTED;
                    if(bottom == UNSELECTED || (colour != bottom / 26 && value == (bottom % 13) - 1)) {
                        moveCards(&board->hand, new_stack, 0);
                    }
                }

                if(board->return_hand != UNSELECTED) {
                    CardStack *return_stack = &board->stacks[board->return_hand];
                    moveCards(&board->hand, return_stack, 0);
                    if(return_stack->card_count == 0) {
                        exposeCard(return_stack, board);
                    }
                    board->return_hand = UNSELECTED;
                }
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

        glUniform1i(obj_shad.selected, board.selected_card);
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
