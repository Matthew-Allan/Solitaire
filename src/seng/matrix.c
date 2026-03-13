#include <seng/matrix.h>

#include <stdio.h>

#define matmnOppBdy(opp, mat, out, m, n, count) \
    for(size_t c = 0; c < count; c++, out += m * n, mat += m * n) { \
        for(size_t i = 0; i < m; i++) { \
            for(size_t j = 0; j < n; j++) { \
                matijr(out, i, j, m) = matijr(mat, i, j, m) opp; \
            } \
        } \
    }

void matmnAddSlr(const float *mat, float scalar, float *out, size_t m, size_t n, size_t count) { matmnOppBdy(+ scalar, mat, out, m, n, count); }

void mat2AddSlr(const mat2 mat, float scalar, mat2 out, size_t count) { matmnAddSlr(marr(mat), scalar, marr(out), 2, 2, count); }
void mat3AddSlr(const mat3 mat, float scalar, mat3 out, size_t count) { matmnAddSlr(marr(mat), scalar, marr(out), 3, 3, count); }
void mat4AddSlr(const mat4 mat, float scalar, mat4 out, size_t count) { matmnAddSlr(marr(mat), scalar, marr(out), 4, 4, count); }

void matmnMltSlr(const float *mat, float scalar, float *out, size_t m, size_t n, size_t count) { matmnOppBdy(* scalar, mat, out, m, n, count); }

void mat2MltSlr(const mat2 mat, float scalar, mat2 out, size_t count) { matmnMltSlr(marr(mat), scalar, marr(out), 2, 2, count); }
void mat3MltSlr(const mat3 mat, float scalar, mat3 out, size_t count) { matmnMltSlr(marr(mat), scalar, marr(out), 3, 3, count); }
void mat4MltSlr(const mat4 mat, float scalar, mat4 out, size_t count) { matmnMltSlr(marr(mat), scalar, marr(out), 4, 4, count); }

void matmnDivSlr(const float *mat, float scalar, float *out, size_t m, size_t n, size_t count) { matmnOppBdy(/ scalar, mat, out, m, n, count); }

void mat2DivSlr(const mat2 mat, float scalar, mat2 out, size_t count) { matmnDivSlr(marr(mat), scalar, marr(out), 2, 2, count); }
void mat3DivSlr(const mat3 mat, float scalar, mat3 out, size_t count) { matmnDivSlr(marr(mat), scalar, marr(out), 3, 3, count); }
void mat4DivSlr(const mat4 mat, float scalar, mat4 out, size_t count) { matmnDivSlr(marr(mat), scalar, marr(out), 4, 4, count); }

void matmnAdd(const float *mat_a, const float *mat_b, float *out, size_t m, size_t n, size_t count) { matmnOppBdy(+ matijr(mat_b, i, j, m), mat_a, out, m, n, count); }

void mat2Add(const mat2 mat_a, const mat2 mat_b, mat2 out, size_t count) { matmnAdd(marr(mat_a), marr(mat_b), marr(out), 2, 2, count); }
void mat3Add(const mat3 mat_a, const mat3 mat_b, mat3 out, size_t count) { matmnAdd(marr(mat_a), marr(mat_b), marr(out), 3, 3, count); }
void mat4Add(const mat4 mat_a, const mat4 mat_b, mat4 out, size_t count) { matmnAdd(marr(mat_a), marr(mat_b), marr(out), 4, 4, count); }

void matmnSub(const float *mat_a, const float *mat_b, float *out, size_t m, size_t n, size_t count) { matmnOppBdy(- matijr(mat_b, i, j, m), mat_a, out, m, n, count); }

void mat2Sub(const mat2 mat_a, const mat2 mat_b, mat2 out, size_t count) { matmnSub(marr(mat_a), marr(mat_b), marr(out), 2, 2, count); }
void mat3Sub(const mat3 mat_a, const mat3 mat_b, mat3 out, size_t count) { matmnSub(marr(mat_a), marr(mat_b), marr(out), 3, 3, count); }
void mat4Sub(const mat4 mat_a, const mat4 mat_b, mat4 out, size_t count) { matmnSub(marr(mat_a), marr(mat_b), marr(out), 4, 4, count); }

void matmnMltMatnp(const float *mat_a, const float *mat_b, float *out, size_t m, size_t n, size_t p, size_t count) {
    matmn(m, p, temp);
    for(size_t c = 0; c < count; c++, out += m * p, mat_a += m * n) {
        for(size_t i = 0; i < m; i++) {
            for(size_t j = 0; j < p; j++) {
                float val = 0;
                for(size_t k = 0; k < n; k++) {
                    val += matijr(mat_a, i, k, m) * matijr(mat_b, k, j, n);
                }
                matij(temp, i, j) = val;
            }
        }
        for(size_t i = 0; i < m; i++) {
            for(size_t j = 0; j < p; j++) {
                matijr(out, i, j, m) = matij(temp, i, j);
            }
        }
    }
}

void mat2Mlt(const mat2 mat_a, const mat2 mat_b, mat2 out, size_t count) { matmnMltMatnp(marr(mat_a), marr(mat_b), marr(out), 2, 2, 2, count); }
void mat3Mlt(const mat3 mat_a, const mat3 mat_b, mat3 out, size_t count) { matmnMltMatnp(marr(mat_a), marr(mat_b), marr(out), 3, 3, 3, count); }
void mat4Mlt(const mat4 mat_a, const mat4 mat_b, mat4 out, size_t count) { matmnMltMatnp(marr(mat_a), marr(mat_b), marr(out), 4, 4, 4, count); }

#define matnMltVecBdy(mat, vec, out, n, count) \
    vec3 temp; \
    for(size_t c = 0; c < count; c++, vec += n, out += n) { \
        cpyVec3(vec, temp); \
        for(int i = 0; i < n; i++) { \
            out[i] = 0; \
            for(int j = 0; j < n; j++) { \
                out[i] += temp[j] * mat[j][i]; \
            } \
        } \
    }

void mat2MltVec(const mat2 mat, const vec2 vec, vec2 out, size_t count) { matnMltVecBdy(mat, vec, out, 2, count) }
void mat3MltVec(const mat3 mat, const vec3 vec, vec3 out, size_t count) { matnMltVecBdy(mat, vec, out, 3, count) }
void mat4MltVec(const mat4 mat, const vec4 vec, vec4 out, size_t count) { matnMltVecBdy(mat, vec, out, 4, count) }

#define matnTransposeBdy(mat, out, n, count) \
    float temp; \
    float *curr_mat = marr(mat), *curr_out = marr(out); \
    for(size_t c = 0; c < count; c++, curr_out += n * n, curr_mat += n * n) { \
        for(int i = 1; i < n; i++) { \
            for(int j = 0; j < i; j++) { \
                temp = matijr(curr_mat, i, j, n); \
                matijr(curr_out, i, j, n) = matijr(curr_mat, j, i, n); \
                matijr(curr_out, j, i, n) = temp; \
            } \
        } \
    }

void mat2Transpose(const mat2 mat, mat2 out, size_t count) { matnTransposeBdy(mat, out, 2, count) }
void mat3Transpose(const mat3 mat, mat3 out, size_t count) { matnTransposeBdy(mat, out, 3, count) }
void mat4Transpose(const mat4 mat, mat4 out, size_t count) { matnTransposeBdy(mat, out, 4, count) }

void printMatmn(const float *mat, size_t m, size_t n, size_t count) {
    for(size_t c = 0; c < count; c++, mat += m * n) {
        for(size_t i = 0; i < m; i++) {
            for(size_t j = 0; j < n; j++) {
                printf("%f ", matijr(mat, i, j, m));
            }
            printf("\n");
        }
        printf(c == count - 1 ? "" : "------------------\n");
    }
}

void printMat2(const mat2 mat, size_t count) { printMatmn(marr(mat), 2, 2, count); }
void printMat3(const mat3 mat, size_t count) { printMatmn(marr(mat), 3, 3, count); }
void printMat4(const mat4 mat, size_t count) { printMatmn(marr(mat), 4, 4, count); }