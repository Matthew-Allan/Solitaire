#include <seng/program.h>
#include <seng/shader.h>
#include <seng/camera.h>
#include <seng/vaos.h>
#include <seng/textures.h>

#include "objectshad.h"

int card = 0;
int suit = 0;

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
            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                card++;
                card %= 13;
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                suit++;
                suit %= 4;
            }
        }
    }
    return 0;
}

// Run the game and gameloop in the given window.
int runGame(Program *program) {
    // Set clear colour to black.
    glClearColor(0.00, 0.00, 0.00, 1.0f);

    ObjectShader obj_shad;
    if(buildObjectShader(&obj_shad) < 0) {
        return -1;
    }

    VertexArrObj card_vao;
    boxVAO_2D(&card_vao, (62.f / 98.f) * 2, 2, -1, -1, 98, 62);

    GLuint atlas; 
    if(loadTex("textures/atlas.png", &atlas, 0, GL_RGBA8UI, GL_RGBA_INTEGER) < 0) {
        return -1;
    }
    glUseProgram(obj_shad.program);
    glUniform1i(obj_shad.atlas, 0);

    Camera cam;
    vec3 pos = vec3(0, 0, 0);
    initCam(&cam, pos);

    // float aspect = (float) program->width / program->height;

    // Main game loop
    while(program->running) {
        pollEvents(program);

        // Clear the screen and draw the grid to the screen.
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(obj_shad.program);
        glUniform1i(obj_shad.card, card + (suit * 13));

        glBindTexture(GL_TEXTURE_2D, atlas);
        drawVAO(&card_vao);

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
