#include <seng/camera.h>

#include <stddef.h>

void initCam(Camera *cam, const vec3 pos) {
    cam->fov = M_PI_4;
    initTrans(&cam->trans, pos, NULL);
}

void uploadCamMat2D(const Camera *cam, GLuint camLoc, float width, float height) {
    mat4 cam_mat;
    invTransMat(&cam->trans, cam_mat);

    mat4 projection = orthoMat(0, width, 0, height, 1, -1);
    mat4Mlt(projection, cam_mat, cam_mat, 1);

    glUniformMatrix4fv(camLoc, 1, GL_FALSE, marr(cam_mat));
}

void uploadCamMat(const Camera *cam, GLuint camLoc, float aspect) {
    mat4 cam_mat;
    invTransMat(&cam->trans, cam_mat);

    mat4 projection = perspMat(cam->fov, aspect, 1, 10000);
    mat4Mlt(projection, cam_mat, cam_mat, 1);

    glUniformMatrix4fv(camLoc, 1, GL_FALSE, marr(cam_mat));
}

#define invOrthoMat(left, right, bottom, top, near, far) mat4( \
    vec4((right-left)/2.0, 0, 0, 0), \
    vec4(0, (top-bottom)/2.0, 0, 0), \
    vec4(0, 0, (far-near)/2.0, 0), \
    vec4((right+left)/2.0, (top+bottom)/2.0, (far+near)/2.0, 1) \
)

void screenToWorld(const Camera *cam, float *x, float *y, float width, float height, float s_width, float s_height) {
    mat4 cam_mat;
    transMat(&cam->trans, cam_mat);

    mat4 proj = invOrthoMat(0, width, 0, height, 1, -1);
    mat4Mlt(cam_mat, proj, cam_mat, 1);

    vec4 pos = vec4((2.0f * *x) / s_width - 1.0f, 1.0f - (2.0f * *y) / s_height, 0, 1);
    mat4MltVec(cam_mat, pos, pos, 1);
    *x = pos[vecX];
    *y = pos[vecY];
}