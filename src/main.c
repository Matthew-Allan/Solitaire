#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>

#include <stdlib.h>
#include <time.h>

#include "objectshad.h"

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
    uint8_t cards_left;
    uint8_t order[52];
} Board;

int cardVAO(VertexArrObj *card_vao) {
    boxVAO_2D(card_vao, (62.f / 98.f), 1, 0, 0, 98, 62);
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
}

void addCard(Card *cards, uint8_t *count, int card, float x, float y) {
    Card *card_pos = cards + *count;
    card_pos->card = card;
    card_pos->y = y;
    card_pos->x = x;
    (*count)++;
}

void drawBoard(Board *board, VertexArrObj *vao, GLuint atlas) {
    Card cards[52];

    uint8_t count = 0;
    for(int s = 0; s < 7; s++) {
        float x = 0.25 + (s * 0.6875);
        float y = 3.5;
        for(int b = 0; b < board->stacks[s].upside_down; b++, y -= 0.25) {
            addCard(cards, &count, 64, x, y);
        }
        for(int c = 0; c < board->stacks[s].card_count; c++, y -= 0.25) {
            addCard(cards, &count, board->stacks[s].cards[c], x, y);
        }
    }
    
    updateVBO(vao, cards, sizeof(cards));

    glBindTexture(GL_TEXTURE_2D, atlas);
    drawVAOInstanced(vao, count);
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

    Board board;

    resetBoard(&board);

    // Main game loop
    while(program->running) {
        pollEvents(program);

        // Clear the screen and draw the grid to the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(obj_shad.program);

        float height = 5;
        float width = windowAspect(program) * height;
        uploadCamMat2D(&cam, obj_shad.camera, width, height);
        
        drawBoard(&board, &card_vao, atlas);

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
