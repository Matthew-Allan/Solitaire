#ifndef VECTOR_H
#define VECTOR_H

#include <sys/types.h>

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];

#define vecX 0
#define vecY 1
#define vecZ 2
#define vecW 3

#define vec2(x, y) {(x), (y)}
#define vec3(x, y, z) {(x), (y), (z)}
#define vec4(x, y, z, w) {(x), (y), (z), (w)}

#define fvec2(v) vec2(v, v)
#define fvec3(v) vec3(v, v, v)
#define fvec4(v) vec4(v, v, v, v)

#define varr(vec) ((float *) (vec))

static const vec3 UP_VEC = vec3(0, 1, 0);
static const vec3 DOWN_VEC = vec3(0, -1, 0);

static const vec3 BACKWARD_VEC = vec3(0, 0, 1);
static const vec3 FORWARD_VEC = vec3(0, 0, -1);

static const vec3 RIGHT_VEC = vec3(1, 0, 0);
static const vec3 LEFT_VEC = vec3(-1, 0, 0);

static const vec3 ORIGIN_VEC = vec3(0, 0, 0);

#define X_AXIS RIGHT_VEC
#define Y_AXIS UP_VEC
#define Z_AXIS BACKWARD_VEC

void vec2AddSlr(const vec2 vec, float scalar, vec2 out, size_t count);
void vec3AddSlr(const vec3 vec, float scalar, vec3 out, size_t count);
void vec4AddSlr(const vec4 vec, float scalar, vec4 out, size_t count);

void vec2MltSlr(const vec2 vec, float scalar, vec2 out, size_t count);
void vec3MltSlr(const vec3 vec, float scalar, vec3 out, size_t count);
void vec4MltSlr(const vec4 vec, float scalar, vec4 out, size_t count);

void vec2DivSlr(const vec2 vec, float scalar, vec2 out, size_t count);
void vec3DivSlr(const vec3 vec, float scalar, vec3 out, size_t count);
void vec4DivSlr(const vec4 vec, float scalar, vec4 out, size_t count);

void vec2Add(const vec2 vec_a, const vec2 vec_b, vec2 out, size_t count);
void vec3Add(const vec3 vec_a, const vec3 vec_b, vec3 out, size_t count);
void vec4Add(const vec4 vec_a, const vec4 vec_b, vec4 out, size_t count);

void vec2Sub(const vec2 vec_a, const vec2 vec_b, vec2 out, size_t count);
void vec3Sub(const vec3 vec_a, const vec3 vec_b, vec3 out, size_t count);
void vec4Sub(const vec4 vec_a, const vec4 vec_b, vec4 out, size_t count);

void vec2Mlt(const vec2 vec_a, const vec2 vec_b, vec2 out, size_t count);
void vec3Mlt(const vec3 vec_a, const vec3 vec_b, vec3 out, size_t count);
void vec4Mlt(const vec4 vec_a, const vec4 vec_b, vec4 out, size_t count);

void vec2Div(const vec2 vec_a, const vec2 vec_b, vec2 out, size_t count);
void vec3Div(const vec3 vec_a, const vec3 vec_b, vec3 out, size_t count);
void vec4Div(const vec4 vec_a, const vec4 vec_b, vec4 out, size_t count);

float vec2Dot(const vec2 vec_a, const vec2 vec_b);
float vec3Dot(const vec3 vec_a, const vec3 vec_b);
float vec4Dot(const vec4 vec_a, const vec4 vec_b);

float vec2Len(const vec2 vec);
float vec3Len(const vec3 vec);
float vec4Len(const vec4 vec);

void setVec2(vec2 vec, float x, float y);
void setVec3(vec3 vec, float x, float y, float z);
void setVec4(vec4 vec, float x, float y, float z, float w);

void cpyVec2(const vec2 src, vec2 dst);
void cpyVec3(const vec3 src, vec3 dst);
void cpyVec4(const vec4 src, vec4 dst);

void vec2Norm(const vec2 vec, vec2 out);
void vec3Norm(const vec3 vec, vec3 out);
void vec4Norm(const vec4 vec, vec4 out);

void crossProduct(const vec3 vec_a, const vec3 vec_b, vec3 out);

void printVec2(const vec2 vec, size_t count);
void printVec3(const vec3 vec, size_t count);
void printVec4(const vec4 vec, size_t count);

#endif