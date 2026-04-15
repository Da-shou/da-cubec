#include <stdbool.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include "shader.h"
#include "utils/shader_utils.h"

void shader_init(shader_t* shader, const char* vertex_filepath, const char* fragment_filepath) {
    shader->id = make_shader(vertex_filepath, fragment_filepath);
}

void shader_use(const shader_t* shader) {
    glUseProgram(shader->id);
}

void shader_destroy(const shader_t* shader) {
    glDeleteProgram(shader->id);
}

void shader_set_bool(const shader_t* shader, const char* name, const bool value) {
    glUniform1i(glGetUniformLocation(shader->id, name), (int)value);
}

void shader_set_int(const shader_t* shader, const char* name, const int value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void shader_set_float(const shader_t* shader, const char* name, const float value) {
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}

void shader_set_mat4(const shader_t* shader, const char* name, mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, (float*)value);
}

void shader_set_vec3(const shader_t* shader, const char* name, vec3 value) {
    glUniform3fv(glGetUniformLocation(shader->id, name), 1, value);
}
