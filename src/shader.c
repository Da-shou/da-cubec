#include <stdbool.h>
#include <glad/gl.h>

#include <shader.h>
#include <utils/shader_utils.h>

void shader_init(shader_t* s, const char* vertex_filepath,
                 const char* fragment_filepath) {
	s->id = make_shader(vertex_filepath, fragment_filepath);
}

void shader_use(shader_t *s) {
	glUseProgram(s->id);
}

void shader_destroy(shader_t *s) {
	glDeleteProgram(s->id);
}

void shader_set_bool(shader_t *s, const char *name, bool value) {
	glUniform1i(glGetUniformLocation(s->id, name), (int)value);
}

void shader_set_int(shader_t *s, const char *name, int value) {
	glUniform1i(glGetUniformLocation(s->id, name), value);
}

void shader_set_float(shader_t *s, const char *name, float value) {
	glUniform1f(glGetUniformLocation(s->id, name), value);
}
