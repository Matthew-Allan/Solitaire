#ifndef TEXTURES_H
#define TEXTURES_H

#include <glad/glad.h>

GLuint loadTex(const char *path, GLuint *tex, uint8_t active_tex, GLenum i_format, GLenum format);

#endif