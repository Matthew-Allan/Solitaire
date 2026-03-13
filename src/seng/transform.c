#include <seng/transform.h>

#include <stdlib.h>

void transComponents(const Transform *trans, quaternion rot, vec3 disp) {
    quaternion parent_rot = vec4(0, 0, 0, 1);
    vec3 parent_disp = vec3(0, 0, 0);
    if(trans->parent != NULL) {
        transComponents(trans->parent, parent_rot, parent_disp);
    }
    quatMltVec(parent_rot, trans->offset, disp);
    vec3Add(disp, parent_disp, disp, 1);
    quatMlt(parent_rot, trans->orientation, rot);
}

void outputMat(const mat3 rot, const vec3 disp, mat4 mat) {
    cpyVec3(rot[matI], mat[matI]);
    cpyVec3(rot[matJ], mat[matJ]);
    cpyVec3(rot[matK], mat[matK]);
    
    cpyVec3(disp, mat[matL]);

    mat[matI][vecW] = 0;
    mat[matJ][vecW] = 0;
    mat[matK][vecW] = 0;
    mat[matL][vecW] = 1;
}

void invTransMat(const Transform *trans, mat4 mat) {
    quaternion rotation; vec3 displacement;
    transComponents(trans, rotation, displacement);
    quatConj(rotation, rotation);
    
    quatMltVec(rotation, displacement, displacement);
    vec3MltSlr(displacement, -1, displacement, 1);
    
    mat3 rot_mat;
    quatRotationMat(rotation, rot_mat);

    outputMat(rot_mat, displacement, mat);
}

void transMat(const Transform *trans, mat4 mat) {
    quaternion rotation; vec3 displacement;
    transComponents(trans, rotation, displacement);

    mat3 rot_mat;
    quatRotationMat(rotation, rot_mat);

    outputMat(rot_mat, displacement, mat);
}

void initTrans(Transform *trans, const vec3 offset, const Transform *parent) {
    cpyVec3(offset, trans->offset);
    trans->parent = parent;
    setVec4(trans->orientation, 0, 0, 0, 1);
}

void uploadTransMat(const Transform *trans, GLuint modelLoc) {
    mat4 model;
    transMat(trans, model);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, marr(model));
}

void parentTrans(Transform *trans, const Transform *parent) {
    trans->parent = parent;
}

void orphanTrans(Transform *trans) {
    trans->parent = NULL;
}

void orphanTransAt(Transform *trans) {
    transComponents(trans, trans->orientation, trans->offset);
    trans->parent = NULL;
}