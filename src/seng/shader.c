#include <seng/shader.h>

#include <stdio.h>
#include <stdlib.h>

#include <seng/files.h>

// Compile a shader at a given path.
int compileShader(const char *path, GLenum shader_type, GLuint *shader) {
    printf("Compiling %s...\n", path);

    // Get source code.
    const char *shad_src = getFileText(path);
    if(!shad_src) {
        return -1;
    }

    // Create and compile the shader.
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &shad_src, NULL);
    glCompileShader(*shader);

    // Free the source code buffer.
    free((void *) shad_src);

    // Check if compilation was successfull.
    int success;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(*shader, 512, NULL, infoLog);
        printf("Shader compilation failed: %s\n", infoLog);
        return -1;
    }

    return 0;
}

// Build a shader given a path to the vertex and fragment shader source files.
int buildShader(GLuint *id, const char *vert_path, const char *frag_path, const char *geom_path) {
    // Compile the vertex and fragment shaders.
    GLuint vert_shad, frag_shad, geom_shad;
    if(
        compileShader(vert_path, GL_VERTEX_SHADER, &vert_shad) < 0 ||
        compileShader(frag_path, GL_FRAGMENT_SHADER, &frag_shad) < 0
    ) {
        return -1;
    }

    if(geom_path != NULL && compileShader(geom_path, GL_GEOMETRY_SHADER, &geom_shad) < 0) {
        return -1;
    }

    // Create the shader program.
    *id = glCreateProgram();

    // Attach and link the shaders in the program.
    glAttachShader(*id, vert_shad);
    glAttachShader(*id, frag_shad);
    if(geom_path != NULL) {
        glAttachShader(*id, geom_shad);
    }
    glLinkProgram(*id);

    // Check for errors.
    int success;
    glGetProgramiv(*id, GL_LINK_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(*id, 512, NULL, infoLog);
        printf("Shader linking failed: %s\n", infoLog);
        return -1;
    }

    // Clear up the shaders.
    glDeleteShader(vert_shad);
    glDeleteShader(frag_shad);
    if(geom_path != NULL) {
        glDeleteShader(geom_shad);
    }

    return 0;
}