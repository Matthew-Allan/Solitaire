#ifndef VOAS_H
#define VOAS_H

#include <glad/glad.h>
#include <stddef.h>

typedef GLuint VertexBufObj;
typedef GLuint ElementBufObj;

typedef struct VertexArrObj{
    GLuint id;
    ElementBufObj ebo;
    VertexBufObj vbo;
    int faces;
} VertexArrObj;

int loadVAO(VertexArrObj *station_VAO, const char *path);
int loadVAOs(VertexArrObj *station_VAOs, const char **paths, size_t count);
void drawVAO(const VertexArrObj *VAO);
void drawVAOInstanced(const VertexArrObj *VAO, size_t count);
int boxVAO_2D(VertexArrObj *VAO, float width, float height, float bottom, float left, float uv_h, float uv_w);

#endif