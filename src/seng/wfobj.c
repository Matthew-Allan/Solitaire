#include <seng/wfobj.h>
#include <seng/files.h>
#include <seng/vector.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define DFLT_LIST_SIZE 12

typedef struct Vec3List {
    vec3 *data;
    size_t count;
    size_t size;
} Vec3List;

typedef struct OBJReader {
    Vec3List vecs;
    Vec3List norms;
    Vec3List faces;
    unsigned int vecs_p_face : 4;
    unsigned int nv : 1;
    unsigned int uv : 1;
} OBJReader;

typedef int(*LineReader)(FILE *fptr, OBJReader *reader);

int extendVec3List(Vec3List *list) {
    int init = list->size == 0;

    size_t new_size = init ? DFLT_LIST_SIZE : list->size * 2;
    vec3 *new_data = malloc(new_size * sizeof(vec3));
    if(new_data == NULL) {
        printf("Couldn't allocate memory for vector list.\n");
        return -1;
    }

    if(!init) {
        memcpy(new_data, list->data, list->size * sizeof(vec3));
        free(list->data);
    }

    list->data = new_data;
    list->size = new_size;
    return 0;
}

int initVec3List(Vec3List *list) {
    list->size = 0;
    list->count = 0;
    extendVec3List(list);
    return 0;
}

void closeVec3List(Vec3List *list) {
    free(list->data);
}

int addVec3ToList(Vec3List *list, const vec3 vec) {
    if(list->count == list->size && extendVec3List(list) == -1) {
        return -1;
    }
    cpyVec3(vec, list->data[list->count]);
    list->count++;
    return 0;
}

int initOBJReader(OBJReader *reader, WF_OBJ_Flags flags) {
    if(initVec3List(&reader->faces) == -1) {
        return -1;
    }
    if(initVec3List(&reader->norms) == -1) {
        return -1;
    }
    if(initVec3List(&reader->vecs) == -1) {
        return -1;
    }
    
    reader->nv = FLAGS_SET(flags, WF_OBJ_HAS_NORMS);
    reader->uv = FLAGS_SET(flags, WF_OBJ_HAS_UVS);
    reader->vecs_p_face = (1 + reader->nv + reader->uv) * 3;
    return 0;
}

float *closeOBJReader(OBJReader *reader, size_t *face_size, size_t *face_count) {
    closeVec3List(&reader->norms);
    closeVec3List(&reader->vecs);

    *face_count = reader->faces.count / reader->vecs_p_face;
    *face_size = sizeof(vec3) * reader->vecs_p_face;

    return varr(reader->faces.data);
}

int readVec(FILE *fptr, OBJReader *reader) {
    vec3 vec = fvec3(0);
    int count = fscanf(fptr, "v %f %f %f\n", vec, vec+1, vec+2);
    if(count != 3) {
        return count == EOF ? EOF : 0;
    }
    return addVec3ToList(&reader->vecs, vec) == -1? EOF : 1;
}

int readNorm(FILE *fptr, OBJReader *reader) {
    vec3 vec = fvec3(0);
    int count = fscanf(fptr, "vn %f %f %f\n", vec, vec+1, vec+2);
    if(count != 3) {
        return count == EOF ? EOF : 0;
    }
    return addVec3ToList(&reader->norms, vec) == -1? EOF : 1;
}

int addFace(OBJReader *reader, const int vec[3], const int norm[3]) {
    for(int i = 0; i < 3; i++) {
        if(addVec3ToList(&reader->faces, reader->vecs.data[vec[i] - 1])) {
            return EOF;
        }
        if(reader->nv) {
            float *norm_data = norm == NULL ? varr(ORIGIN_VEC) : varr(reader->norms.data[norm[i] - 1]);
            if(addVec3ToList(&reader->faces, norm_data)) {
                return EOF;
            }
        }
    }
    return 1;
}

int readFaceNorm(FILE *fptr, OBJReader *reader) {
    int vec[3], norm[3];
    int count = fscanf(fptr, "f %d//%d %d//%d %d//%d\n", vec, norm, vec+1, norm+1, vec+2, norm+2);
    if(count != 6) {
        return count == EOF ? EOF : 0;
    }
    return addFace(reader, vec, norm);
}

int readFace(FILE *fptr, OBJReader *reader) {
    int vec[3];
    int count = fscanf(fptr, "f %d %d %d\n", vec, vec+1, vec+2);
    if(count != 3) {
        return count == EOF ? EOF : 0;
    }
    return addFace(reader, vec, NULL);
}

const LineReader line_readers[] = {readVec, readNorm, readFaceNorm, readFace};

#define line_reader_count (sizeof(line_readers) / sizeof(LineReader))

int parseLine(FILE *fptr, OBJReader *reader) {
    long start = ftell(fptr);
    for(size_t i = 0; i < line_reader_count; i++) {
        int out = line_readers[i](fptr, reader);
        if(out) {
            return out != EOF;
        }
        fseek(fptr, start, SEEK_SET);
    }

    int c;
    while((c = fgetc(fptr)) != '\n' && c != EOF) {}
    return c != EOF;
}

float *loadObjectData(const char *path, WF_OBJ_Flags flags, size_t *face_size, size_t *face_count) {
    FILE *file = openFile(path);
    if(file == NULL) {
        printf("Couldn't open object file \"%s\"\n", strerror(errno));
        return NULL;
    }

    OBJReader reader;
    if(initOBJReader(&reader, flags) == -1) {
        fclose(file);
        return NULL;
    }

    while(parseLine(file, &reader)) {}

    fclose(file);
    return closeOBJReader(&reader, face_size, face_count);
}
