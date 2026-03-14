#ifndef PROGRAM_H
#define PROGRAM_H

#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "config.h"

typedef struct Program {
    SDL_Window *window;
    const Uint8 *keystate;
    uint64_t prev_time;
    uint64_t delta_time;
    float frames;
    int width;
    int height;
    int running : 1;
    int fullscreen : 1;
} Program;

SDL_Window *createWindow();
int createProgram(Program *program);
void waitForFrame(Program *program);
void updateViewport(Program *program);
void setFullscreen(Program *program, int fullscreen);
void toggleFullscreen(Program *program);

#define windowAspect(program) ((float) ((Program *)(program)->width) / program->height)

#endif