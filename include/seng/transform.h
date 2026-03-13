#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glad/glad.h>

#include "matrix.h"
#include "quaternion.h"

typedef struct Transform {
    const struct Transform *parent;
    quaternion orientation;
    vec3 offset;
} Transform;

void transMat(const Transform *trans, mat4 mat);
void invTransMat(const Transform *trans, mat4 mat);

void initTrans(Transform *trans, const vec3 offset, const Transform *parent);

void uploadTransMat(const Transform *trans, GLuint modelLoc);

void parentTrans(Transform *trans, const Transform *parent);
void orphanTrans(Transform *trans);
void orphanTransAt(Transform *trans);

#endif