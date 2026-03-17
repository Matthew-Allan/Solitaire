#include <seng/program.h>

#include <stdio.h>

// Create an SDL window and an OpenGL context.
SDL_Window *createWindow() {
    // Init SDL.
    printf("Initing SDL.\n");
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not init SDL: %s\n", SDL_GetError());
        return NULL;
    }

    printf("Initing SDL image.\n");
    if(IMG_Init(IMG_INIT_PNG) < 0) {
        printf("Could not init SDL image: %s\n", IMG_GetError());
        return NULL;
    }

    // Set profile to core and version.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR_VER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR_VER);

    // Create the SDL window.
    printf("Creating window.\n");
    SDL_Window *window = SDL_CreateWindow(
        PROGRAM_NAME,                                   // Window title
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // Center the x and y position on the screen.
        SCREEN_WIDTH, SCREEN_HEIGHT,                    // Set height and width.
        WINDOW_FLAGS                                    // (Flags) Open window useable with OpenGL context.
    );

    // Check that the window was created.
    if(!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return NULL;
    }

    // Create a GL context.
    printf("Creating context.\n");
    SDL_GL_CreateContext(window);

    // Give glad the function loader specific to the OS.
    printf("Loading GL loader.\n");
    if(!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        printf("Failed to load GL\n");
        return NULL;
    }

    // Return a pointer to the window.
    return window;
}

void updateWinDims(Program *program) {
    SDL_GetWindowSizeInPixels(program->window, &program->width, &program->height);
    glViewport(0, 0, program->width, program->height);
    program->aspect = (float) program->width / program->height;
}

void getMouseNDC(Program *program, float *ndc_x, float *ndc_y) {
    int x, y;
    SDL_GetMouseState(&x, &y);

    *ndc_x = (4.0f * x) / program->width - 1.0f;
    *ndc_y = 1.0f - (4.0f * y) / program->height;
}

int createProgram(Program *program) {
    program->window = createWindow();
    if(program->window == NULL) {
        return -1;
    }
    updateWinDims(program);
    program->running = 1;
    program->delta_time = 0;
    program->prev_time = SDL_GetTicks64();
    program->fullscreen = 1;
    program->keystate = SDL_GetKeyboardState(NULL);
    return 0;
}

float getFps(uint64_t *current_time, Program *program) {
    *current_time = SDL_GetTicks64();
    program->delta_time = *current_time - program->prev_time;
    if(program->delta_time == 0) {
        return fps + 1;
    }
    program->frames = 1000.f / program->delta_time;
    return program->frames;
}

void setFullscreen(Program *program, int fullscreen) {
    if(program->fullscreen == fullscreen) {
        return;
    }
    SDL_SetWindowFullscreen(program->window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    program->fullscreen = fullscreen;
}

void toggleFullscreen(Program *program) {
    setFullscreen(program, !program->fullscreen);
}

// Loop until enough time has passed for the fps to be correct.
void waitForFrame(Program *program) {
    uint64_t time_value;
    #if cap_fps==1
    while(getFps(&time_value, program) > fps) {
        SDL_Delay(1);
    }
    #else
    getFps(&time_value, program);
    #endif
    // Work out time between last frame and this one.
    program->prev_time = time_value;
    program->keystate = SDL_GetKeyboardState(NULL);
}