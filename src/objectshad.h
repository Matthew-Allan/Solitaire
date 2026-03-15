#ifndef OBJECT_SHAD_H
#define OBJECT_SHAD_H

#include <glad/glad.h>

typedef struct ObjectShader {
    GLuint program;
    GLuint atlas;
    GLuint camera;
    GLuint selected;
} ObjectShader;

#define FRAG_SHADER_PATH "shaders/frag.glsl"
#define VERT_SHADER_PATH "shaders/vert.glsl"

int buildObjectShader(ObjectShader *shader);

#endif