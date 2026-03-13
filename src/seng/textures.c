#include <seng/textures.h>

#include <seng/files.h>

#include <SDL2/SDL_image.h>
#include <stdio.h>

GLuint loadTex(const char *path, GLuint *tex, uint8_t active_tex, GLenum i_format, GLenum format) {
    char *abs_path = getPath(path);
    if(abs_path == NULL) {
        return -1;
    }

    SDL_Surface *surface = IMG_Load(abs_path);
    free(abs_path);

    if(surface == NULL) {
        printf("Failed to load image\n");
        return -1;
    }

    glGenTextures(1, tex);
    glActiveTexture(GL_TEXTURE0 + active_tex);
    glBindTexture(GL_TEXTURE_2D, *tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,              // Target the currently binded GL_TEXTURE_2D.
        0, i_format,                // Mipmap level; Tell GL to store in RGB format.
        surface->w, surface->h, 0,  // Width and height (and border value always set to 0).
        format, GL_UNSIGNED_BYTE,   // Format and datatype of source image.
        surface->pixels             // Pixel data.
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(surface);

    return 0;
}