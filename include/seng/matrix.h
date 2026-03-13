#ifndef MATRIX_H
#define MATRIX_H

#include <sys/types.h>
#include <math.h>

#include "vector.h"

#define matmn(m, n, name) float name[n][m]

typedef matmn(2, 2, mat2);
typedef matmn(3, 3, mat3);
typedef matmn(4, 4, mat4);

#define matI 0
#define matJ 1
#define matK 2
#define matL 3

#define mat2(i, j) {i, j}
#define mat3(i, j, k) {i, j, k}
#define mat4(i, j, k, w) {i, j, k, w}

#define fmat2(v) mat2(fvec2(v), fvec2(v))
#define fmat3(v) mat3(fvec3(v), fvec3(v), fvec3(v))
#define fmat4(v) mat4(fvec4(v), fvec4(v), fvec4(v), fvec4(v))

#define mat2i(a, b) mat2(vec2(a, 0), vec2(0, b))
#define mat3i(a, b, c) mat3(vec3(a, 0, 0), vec3(0, b, 0), vec3(0, 0, c))
#define mat4i(a, b, c, d) mat4(vec4(a, 0, 0, 0), vec4(0, b, 0, 0), vec4(0, 0, c, 0), vec4(0, 0, 0, d))

#define fmat2i(v) mat2i(v, v)
#define fmat3i(v) mat3i(v, v, v)
#define fmat4i(v) mat4i(v, v, v, v)

#define scaleMat(x, y, z) mat4i(x, y, z, 1)
#define DispMat(x, y, z) mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(x, y, z, 1))

#define rotxMat(theta) mat4(vec4(1, 0, 0, 0), vec4(0, cosf(theta), sinf(theta), 0), vec4(0, -sinf(theta), cosf(theta), 0), vec4(0, 0, 0, 1))
#define rotyMat(theta) mat4(vec4(cosf(theta), 0, -sinf(theta), 0), vec4(0, 1, 0, 0), vec4(sinf(theta), 0, cosf(theta), 0), vec4(0, 0, 0, 1))
#define rotzMat(theta) mat4(vec4(cosf(theta), sinf(theta), 0, 0), vec4(-sinf(theta), cosf(theta), 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1))

#define rotAxisMat(rx, ry, rz, theta) mat4( \
    vec4((      cosf(theta)) + (rx * rx * (1 - cosf(theta))), (rz *  sinf(theta)) + (ry * rx * (1 - cosf(theta))), (ry * -sinf(theta)) + (rz * rx * (1 - cosf(theta))), 0), \
    vec4((rz * -sinf(theta)) + (rx * ry * (1 - cosf(theta))), (      cosf(theta)) + (ry * ry * (1 - cosf(theta))), (rx *  sinf(theta)) + (rz * ry * (1 - cosf(theta))), 0), \
    vec4((ry *  sinf(theta)) + (rx * rz * (1 - cosf(theta))), (rx * -sinf(theta)) + (ry * rz * (1 - cosf(theta))), (      cosf(theta)) + (rz * rz * (1 - cosf(theta))), 0), \
    vec4(0, 0, 0, 1) \
)

#define orthoMatSp(a, b) (2.f / (a - b))
#define orthoMatTp(a, b) (((float) (a + b)) / (b - a))

#define orthoMat(left, right, bottom, top, near, far) mat4( \
    vec4(orthoMatSp(right, left), 0, 0, 0), \
    vec4(0, orthoMatSp(top, bottom), 0, 0), \
    vec4(0, 0, orthoMatSp(near, far), 0), \
    vec4(orthoMatTp(right, left), orthoMatTp(top, bottom), orthoMatTp(far, near), 1) \
)

#define perspMat(fov, aspect, near, far) mat4( \
    vec4(1.f / (aspect * tanf(fov / 2.f)), 0, 0, 0), \
    vec4(0, 1.f / tanf(fov / 2.f), 0, 0), \
    vec4(0, 0, -(far + near) / (float) (far - near), -1.f), \
    vec4(0, 0, -(2.f * far * near) / (far - near), 0) \
)

#define marr(mat) ((float *) (mat))

#define matijr(mat, i, j, row) (mat[(j * row) + i])
#define matij(mat, i, j) (mat[j][i])

void mat2AddSlr(const mat2 mat, float scalar, mat2 out, size_t count);
void mat3AddSlr(const mat3 mat, float scalar, mat3 out, size_t count);
void mat4AddSlr(const mat4 mat, float scalar, mat4 out, size_t count);

void matmnAddSlr(const float *mat, float scalar, float *out, size_t m, size_t n, size_t count);

void mat2MltSlr(const mat2 mat, float scalar, mat2 out, size_t count);
void mat3MltSlr(const mat3 mat, float scalar, mat3 out, size_t count);
void mat4MltSlr(const mat4 mat, float scalar, mat4 out, size_t count);

void matmnMltSlr(const float *mat, float scalar, float *out, size_t m, size_t n, size_t count);

void mat2DivSlr(const mat2 mat, float scalar, mat2 out, size_t count);
void mat3DivSlr(const mat3 mat, float scalar, mat3 out, size_t count);
void mat4DivSlr(const mat4 mat, float scalar, mat4 out, size_t count);

void matmnDivSlr(const float *mat, float scalar, float *out, size_t m, size_t n, size_t count);

void mat2Add(const mat2 mat_a, const mat2 mat_b, mat2 out, size_t count);
void mat3Add(const mat3 mat_a, const mat3 mat_b, mat3 out, size_t count);
void mat4Add(const mat4 mat_a, const mat4 mat_b, mat4 out, size_t count);

void matmnAdd(const float *mat_a, const float *mat_b, float *out, size_t m, size_t n, size_t count);

void mat2Sub(const mat2 mat_a, const mat2 mat_b, mat2 out, size_t count);
void mat3Sub(const mat3 mat_a, const mat3 mat_b, mat3 out, size_t count);
void mat4Sub(const mat4 mat_a, const mat4 mat_b, mat4 out, size_t count);

void matmnSub(const float *mat_a, const float *mat_b, float *out, size_t m, size_t n, size_t count);

void mat2Mlt(const mat2 mat_a, const mat2 mat_b, mat2 out, size_t count);
void mat3Mlt(const mat3 mat_a, const mat3 mat_b, mat3 out, size_t count);
void mat4Mlt(const mat4 mat_a, const mat4 mat_b, mat4 out, size_t count);

void matmnMltMatnp(const float *mat_a, const float *mat_b, float *out, size_t m, size_t n, size_t p, size_t count);

void mat2MltVec(const mat2 mat, const vec2 vec, vec2 out, size_t count);
void mat3MltVec(const mat3 mat, const vec3 vec, vec3 out, size_t count);
void mat4MltVec(const mat4 mat, const vec4 vec, vec4 out, size_t count);

void mat2Transpose(const mat2 mat, mat2 out, size_t count);
void mat3Transpose(const mat3 mat, mat3 out, size_t count);
void mat4Transpose(const mat4 mat, mat4 out, size_t count);

void printMat2(const mat2 mat, size_t count);
void printMat3(const mat3 mat, size_t count);
void printMat4(const mat4 mat, size_t count);

void printMatmn(const float *mat, size_t m, size_t n, size_t count);

#endif