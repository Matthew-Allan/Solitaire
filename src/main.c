#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>
#include <seng/quaternion.h>

#include "solitaire.h"
#include "cardshader.h"

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
                pickUp(board);
                if(board->hand.card_count > 0) {
                    SDL_ShowCursor(SDL_DISABLE);
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT) {
                SDL_ShowCursor(SDL_ENABLE);
                putDown(board);
            }
            break;
        }
    }
    return 0;
}

int cardVAO(VertexArrObj *card_vao) {
    boxVAO_2D(card_vao, CARD_W, CARD_H, -CARD_H / 2, -CARD_W / 2, CARD_PIXEL_H, CARD_PIXEL_W);
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

        getMouseNDC(program, &board.mouse_wspc_x, &board.mouse_wspc_y);
        screenToWorld(&cam, &board.mouse_wspc_x, &board.mouse_wspc_y);

        checkHovered(&board);
        updateAnims(&board);
        updateCards(&board, cards, &count);

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
    // Create the window
    Program program;
    if(createProgram(&program) == 0) {
        runGame(&program);
    }

    // Quit and return.
    SDL_Quit();
    return 0;
}
