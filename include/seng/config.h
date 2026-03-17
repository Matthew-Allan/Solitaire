#ifndef CONFIG_H
#define CONFIG_H

#define PROGRAM_NAME "Solitaire"

#define SCREEN_WIDTH 1000    // Width of the screen in window points (not in pixels).
#define SCREEN_HEIGHT 600   // Height of the screen in window points (not in pixels).

#define GL_MAJOR_VER 3  // The OpenGL major version number to target.
#define GL_MINOR_VER 3  // The OpenGL minor version number to target.

// Flags for when creating a window. Use OpenGL and allow high DPI.
#define WINDOW_FLAGS SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE

// Set the target FPS.
#define fps 60
#define cap_fps 1

#endif