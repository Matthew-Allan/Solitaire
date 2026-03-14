#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>

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

typedef struct card_data {
    float x;
    float y;
    int card;
} card_data;

int cardVAO(VertexArrObj *card_vao, card_data *data, size_t size) {
    boxVAO_2D(card_vao, (62.f / 98.f), 1, 0, 0, 98, 62);
    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(card_vao->id);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(card_data), (void*)0);
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(card_data), (void*)(2 * sizeof(float)));
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

    card_data cards[52];
    for(int i = 0; i < 52; i++) {
        int r = i / 13;
        int c = i % 13;
        cards[i].card = i % 52;
        cards[i].x = (c) * (62.f / 98.f);
        cards[i].y = (r);
    }

    VertexArrObj card_vao;
    cardVAO(&card_vao, cards, sizeof(cards));

    GLuint atlas; 
    if(loadTex("textures/atlas.png", &atlas, 0, GL_RGBA8UI, GL_RGBA_INTEGER) < 0) {
        return -1;
    }
    glUseProgram(obj_shad.program);
    glUniform1i(obj_shad.atlas, 0);

    Camera cam;
    vec3 pos = vec3(0, 0, 0);
    initCam(&cam, pos);

    // Main game loop
    while(program->running) {
        pollEvents(program);

        // Clear the screen and draw the grid to the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(obj_shad.program);

        uploadCamMat2D(&cam, obj_shad.camera);

        glBindTexture(GL_TEXTURE_2D, atlas);
        drawVAOInstanced(&card_vao, 52);

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
