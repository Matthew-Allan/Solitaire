#ifndef QUATERNION_H
#define QUATERNION_H

#include "matrix.h"

typedef vec4 quaternion;

void quatRotationMat(const quaternion quat, mat3 mat);
void quatFromEuler(quaternion quat, float x, float y, float z);
void quatMlt(const quaternion quat_a, const quaternion quat_b, quaternion out);
void quatConj(const quaternion quat, quaternion out);
void quatMltVec(const quaternion quat, const vec3 vec, vec3 out);

#define quatNorm(quat, out) vec4Norm(quat, out)

#endif