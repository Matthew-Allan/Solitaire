#ifndef WF_OBJ_H
#define WF_OBJ_H

#include <stddef.h>

#define FLAG_MODULE_NAME WF_OBJ

#define FLAG_LIST \
    FLAG_ELEMENT(HAS_NORMS) \
    FLAG_ELEMENT(HAS_UVS) \

#include "flags.h"

#define WF_OBJ_HAS_NRUVS (WF_OBJ_HAS_NORMS | WF_OBJ_HAS_UVS)

float *loadObjectData(const char *path, WF_OBJ_Flags flags, size_t *face_size, size_t *face_count);

#endif