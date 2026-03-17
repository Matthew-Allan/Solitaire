#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include <glad/glad.h>

#include "matrix.h"
#include "transform.h"

typedef struct Camera {
    Transform trans;
    mat4 proj;
    mat4 inv_proj;
} Camera;

void uploadCamMat(const Camera *cam, GLuint camLoc);
void screenToWorld(const Camera *cam, float *x, float *y);

void updatePersp(Camera *cam, float aspect, float fov);
void updateOrtho(Camera *cam, float width, float height);

void initOrthoCam(Camera *cam, const vec3 pos, float width, float height);
void initPerspCam(Camera *cam, const vec3 pos, float aspect, float fov);

#endif