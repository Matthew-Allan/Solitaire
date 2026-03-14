#include "objectshad.h"

#include <stddef.h>

#include <seng/shader.h>

int buildObjectShader(ObjectShader *shader) {
    if(buildShader(&shader->program, VERT_SHADER_PATH, FRAG_SHADER_PATH, NULL) == -1) {
        return -1;
    }

    shader->atlas = glGetUniformLocation(shader->program, "atlas");
    shader->camera = glGetUniformLocation(shader->program, "camera");
    return 0;
}