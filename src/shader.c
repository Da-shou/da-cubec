#include <stdbool.h>
#include <glad/gl.h>

#include <shader.h>
#include <utils/shader_utils.h>

void shader_init(shader_t* s, const char* vertex_filepath,
                 const char* fragment_filepath) {
    s->id = make_shader(vertex_filepath, fragment_filepath);
}

void shader_use(const shader_t* s) {
    glUseProgram(s->id);
}

void shader_destroy(const shader_t* s) {
    glDeleteProgram(s->id);
}

void shader_set_bool(const shader_t* s, const char* name,
                     const bool value) {
    glUniform1i(glGetUniformLocation(s->id, name), (int)value);
}

void shader_set_int(const shader_t* s, const char* name, const int value) {
    glUniform1i(glGetUniformLocation(s->id, name), value);
}

void shader_set_float(const shader_t* s, const char* name,
                      const float value) {
    glUniform1f(glGetUniformLocation(s->id, name), value);
}

void shader_set_mat4(const shader_t* s, const char* name, mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(s->id, name), 1, GL_FALSE,
                       (float*)value);
}
