#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include <glad/glad.h>

#include "matrix.h"
#include "transform.h"

typedef struct Camera {
    Transform trans;
    float fov;
} Camera;

void initCam(Camera *cam, const vec3 pos);

void uploadCamMat(const Camera *cam, GLuint camLoc, float aspect);
void uploadCamMat2D(const Camera *cam, GLuint camLoc, float width, float height);
void screenToWorld(const Camera *cam, float *x, float *y, float width, float height, float s_width, float s_height);
#endif