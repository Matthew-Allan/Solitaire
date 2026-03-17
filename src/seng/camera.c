#include <seng/camera.h>

#include <stddef.h>

void uploadCamMat(const Camera *cam, GLuint camLoc) {
    mat4 cam_mat;
    invTransMat(&cam->trans, cam_mat);
    mat4Mlt(cam->proj, cam_mat, cam_mat, 1);

    glUniformMatrix4fv(camLoc, 1, GL_FALSE, marr(cam_mat));
}

void screenToWorld(const Camera *cam, float *x, float *y) {
    mat4 cam_mat;
    transMat(&cam->trans, cam_mat);
    mat4Mlt(cam_mat, cam->inv_proj, cam_mat, 1);

    vec4 front = vec4(*x, *y, 1, 1);
    vec4 origin = vec4(*x, *y, -1, 1);
    mat4MltVec(cam_mat, front, front, 1);
    mat4MltVec(cam_mat, origin, origin, 1);
    homoVecNorm(front, front);
    homoVecNorm(origin, origin);

    vec4 direction;
    vec4Sub(front, origin, direction, 1);
    if(fabsf(direction[vecZ]) < 0.00001) {
        *x = INFINITY; *y = INFINITY;
    }

    vec4Norm(direction, direction);
    float distance = -origin[vecZ] / direction[vecZ];
    vec4MltSlr(direction, distance, direction, 1);
    vec4Add(origin, direction, origin, 1);
    *x = origin[vecX];
    *y = origin[vecY];
}

void updatePersp(Camera *cam, float aspect, float fov) {
    cpyMat4((mat4) perspMat(fov, aspect, 1, 10000), cam->proj);
    cpyMat4((mat4) invPerspMat(fov, aspect, 1, 10000), cam->inv_proj);
}

void updateOrtho(Camera *cam, float width, float height) {
    cpyMat4((mat4) orthoMat(0, width, 0, height, 1, -1), cam->proj);
    cpyMat4((mat4) invOrthoMat(0, width, 0, height, 1, -1), cam->inv_proj);
}

void initOrthoCam(Camera *cam, const vec3 pos, float width, float height) {
    initTrans(&cam->trans, pos, NULL);
    updateOrtho(cam, width, height);
}

void initPerspCam(Camera *cam, const vec3 pos, float aspect, float fov) {
    initTrans(&cam->trans, pos, NULL);
    updatePersp(cam, aspect, fov);
}