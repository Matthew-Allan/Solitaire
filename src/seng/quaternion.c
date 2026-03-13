#include <seng/quaternion.h>

#define Q(q, a) ((q)[vec ## a])

#define MPRT(a, b) (Q(quat_a, a) * Q(quat_b, b))
#define MROW(b, c, d) (MPRT(W, b) + MPRT(b, W) + MPRT(c, d) - MPRT(d, c))

void quatMlt(const quaternion quat_a, const quaternion quat_b, quaternion out) {
    quaternion temp;
    temp[vecX] = MROW(X, Y, Z);
    temp[vecY] = MROW(Y, Z, X);
    temp[vecZ] = MROW(Z, X, Y);
    temp[vecW] = MPRT(W, W) - vec3Dot(quat_a, quat_b);
    cpyVec4(temp, out);
}

void quatFromEuler(quaternion quat, float x, float y, float z) {
    vec4 x_rot = vec4(sinf(x / 2), 0, 0, cosf(x / 2));
    vec4 y_rot = vec4(0, sinf(y / 2), 0, cosf(y / 2));
    vec4 z_rot = vec4(0, 0, sinf(z / 2), cosf(z / 2));
    quatMlt(x_rot, y_rot, quat);
    quatMlt(quat, z_rot, quat);
    quatNorm(quat, quat);
}

#define OUT(a, b, opp, c, d) (2 * (Q(quat, a) * Q(quat, b) opp Q(quat, c) * Q(quat, d)))
#define CTR(a, b) (1 - OUT(a, a, +, b, b))

void quatRotationMat(const quaternion quat, mat3 mat) {
    setVec3(mat[matI], CTR(Y, Z),          OUT(X, Y, +, W, Z), OUT(X, Z, -, W, Y));
    setVec3(mat[matJ], OUT(X, Y, -, W, Z), CTR(X, Z),          OUT(Y, Z, +, W, X));
    setVec3(mat[matK], OUT(X, Z, +, W, Y), OUT(Y, Z, -, W, X), CTR(X, Y));
}

void quatConj(const quaternion quat, quaternion out) {
    vec4MltSlr(quat, -1, out, 1);
    out[vecW] *= -1;
}

void quatMltVec(const quaternion quat, const vec3 vec, vec3 out) {
    quaternion pure_quat, conj;
    cpyVec3(vec, pure_quat);
    pure_quat[vecW] = 0;

    quatMlt(quat, pure_quat, pure_quat);

    quatConj(quat, conj);
    quatMlt(pure_quat, conj, pure_quat);

    cpyVec3(pure_quat, out);
}