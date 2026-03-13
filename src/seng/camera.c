#include <seng/camera.h>

#include <stddef.h>

void initCam(Camera *cam, const vec3 pos) {
    cam->fov = M_PI_4;
    initTrans(&cam->trans, pos, NULL);
}

void uploadCamMat(const Camera *cam, GLuint camLoc, float aspect) {
    mat4 cam_mat;
    invTransMat(&cam->trans, cam_mat);

    mat4 projection = perspMat(cam->fov, aspect, 1, 10000);
    mat4Mlt(projection, cam_mat, cam_mat, 1);

    glUniformMatrix4fv(camLoc, 1, GL_FALSE, marr(cam_mat));
}