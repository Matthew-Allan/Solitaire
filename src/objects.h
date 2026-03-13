static const char *obj_paths[] = {
    "models/Cube.obj",
    "models/Monkey.obj",
    "models/SpaceStation.obj"
};

#define obj_count (sizeof(obj_paths) / sizeof(char *))

typedef enum Object_Ref {
    OBJ_Cube,
    OBJ_Monkey,
    OBJ_SpaceStation
} Object_Ref;
