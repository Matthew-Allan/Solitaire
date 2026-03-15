#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>
#include <seng/quaternion.h>

#include <stdlib.h>
#include <time.h>

#include "objectshad.h"

#define CARD_WIDTH (62.f / 98.f)
#define CARD_HEIGHT 1

int pollEvents(Program *program) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            program->running = 0;
            break;
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                updateViewport(program);
            }
        case SDL_KEYDOWN:
            if(event.key.keysym.scancode == SDL_SCANCODE_P) {
                toggleFullscreen(program);
            }
        }
    }
    return 0;
}

typedef struct Card {
    float x;
    float y;
    int card;
} Card;

typedef struct CardStack {
    uint8_t upside_down;
    uint8_t card_count;
    uint8_t cards[13];
} CardStack;

typedef struct Board {
    CardStack stacks[7];
    uint8_t selected_card;
    uint8_t cards_left;
    uint8_t order[52];
} Board;

int cardVAO(VertexArrObj *card_vao) {
    boxVAO_2D(card_vao, CARD_WIDTH, CARD_HEIGHT, 0, 0, 98, 62);
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

void exposeCard(CardStack *stack, Board *board) {
    if(stack->upside_down == 0) {
        return;
    }
    stack->upside_down--;
    stack->cards[0] = getCard(board);
    stack->card_count = 1;
    for(int j = 0; j < 3; j++) {
        stack->cards[stack->card_count] = getCard(board);
        stack->card_count++;
    }
}

void resetBoard(Board *board) {
    for(int i = 0; i < 52; i++) {
        board->order[i] = i;
    }
    board->cards_left = 52;
    for(int i = 0; i < 7; i++) {
        board->stacks[i].upside_down = i + 1;
        exposeCard(&board->stacks[i], board);
    }
    board->selected_card = -1;
}

void addCard(Card *cards, uint8_t *count, int card, float x, float y) {
    Card *card_pos = cards + *count;
    card_pos->card = card;
    card_pos->y = y;
    card_pos->x = x;
    (*count)++;
}

void updateCards(Board *board, Card *cards, uint8_t *count) {
    *count = 0;
    for(int s = 0; s < 7; s++) {
        CardStack *stack = &board->stacks[s];
        float x = 0.25 + (s * 0.6875);
        float y = 3.5 - 0.25 * (stack->card_count + stack->upside_down);
        for(int c = stack->card_count - 1; c >= 0; c--, y += 0.25) {
            addCard(cards, count, stack->cards[c], x, y);
        }
        for(int b = 0; b < stack->upside_down; b++, y += 0.25) {
            addCard(cards, count, 64, x, y);
        }
    }
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
    initCam(&cam, pos);
    quatFromEuler(cam.trans.orientation, 0, 0, 0);

    Board board;
    resetBoard(&board);
        
    Card cards[52];
    uint8_t count = 0;

    // Main game loop
    while(program->running) {
        pollEvents(program);

        updateCards(&board, cards, &count);
        float height = 5;
        float width = windowAspect(program) * height;

        int x, y;
        SDL_GetMouseState(&x, &y);
        float w_x = x, w_y = y;
        screenToWorld(&cam, &w_x, &w_y, width, height, program->width / 2, program->height / 2);

        board.selected_card = -1;
        for(int i = 0; i < count; i++) {
            Card *c = &cards[i];
            if(
                w_x >= c->x &&
                w_y >= c->y &&
                w_x <= c->x + CARD_WIDTH &&
                w_y <= c->y + CARD_HEIGHT
            ) {
                board.selected_card = i;
                break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(obj_shad.program);

        uploadCamMat2D(&cam, obj_shad.camera, width, height);

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
