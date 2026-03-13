#include <seng/files.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>

#elif defined(_WIN32)

#include <windows.h>

#else 

#error "Unsupported Operating system (files.c)"

#endif

// Add the relative part to the path.
void catRelPart(const char *rel, char *path) {
    #ifdef _WIN32
    path += strlen(path);
    while(*rel != '\0') {
        *path = *rel == '/' ? '\\' : *rel;
        path++; rel++;
    }
    *path = '\0';
    #else

    strcat(path, rel);
    #endif
}

// Add the absolute part to the path.
int catAbsPart(char *path, size_t max_len) {
    #ifdef __APPLE__
    CFBundleRef main_bundle;
    if(!(main_bundle = CFBundleGetMainBundle())) {
        return -1;
    }

    CFURLRef resourcesURL;
    if(!(resourcesURL = CFBundleCopyResourcesDirectoryURL(main_bundle))) {
        return -1;
    }

    Boolean res = CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8 *) path, max_len - 1);
    CFRelease(resourcesURL);
    if(!res) {
        return -1;
    }

    strcat(path, "/");


    #elif defined(_WIN32)

    if(GetModuleFileNameA(NULL, path, max_len) == 0) {
        return -1;
    }

    char *last_slash = strrchr(path, '\\');
    if(last_slash != NULL) {
        *(last_slash + 1) = '\0';
    }

    #endif

    return 0;
}

// Get the absolute path of the app and add the path value to it.
char *getPath(const char *path) {
    // Allocate memory for the path.
    char *abs_path = malloc(PATH_MAX);
    if(abs_path == NULL) {
        return NULL;
    }

    // If the path starts with a slash, assume its absolute already and just copy into the abs_path.
    if(path != NULL && path[0] == '/') {
        strcpy(abs_path, path);
        return abs_path;
    }

    // Get the absolute part of the path.
    size_t rel_len = path != NULL ? strlen(path) : 0;
    catAbsPart(abs_path, PATH_MAX - rel_len);

    // Get the relative part of the path.
    if(path != NULL) {
        catRelPart(path, abs_path);
    }

    return abs_path;
}

// Open the file given a relative or absolute path.
FILE *openFile(const char *rel) {
    char *abs_path = getPath(rel);
    FILE *file = fopen(abs_path, "rb");
    free(abs_path);
    return file;
}

// Get the contents of a file.
char *getFileText(const char *path) {
    FILE *file = openFile(path);

    if(!file) {
        printf("Couldn't open file \"%s\"\n", strerror(errno));
        return NULL;
    }

    // Get size of the file.
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // Create a buffer for the contents.
    char *contents = (char *) malloc(size + 1);
    if(!contents) {
        printf("Couldn't allocate memory for file buffer.\n");
        return NULL;
    }

    // Add the null terminator.
    contents[size] = '\0';

    // Read the contents into the buffer.
    fread(contents, 1, size, file);

    fclose(file);
    return contents;
}
