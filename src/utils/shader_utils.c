#include <stdio.h>
#include <stdlib.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader_utils.h"
#include "io_utils.h"

unsigned int make_module(const char* filepath, unsigned int module_type) {
        const char* const source = read_file(filepath);

        // Assigning shader and source code to module
        unsigned int shader_module = glCreateShader(module_type);
        glShaderSource(shader_module, 1, &source, NULL);
        glCompileShader(shader_module);

        // Getting the status code of compilation
        int status;
        glGetShaderiv(shader_module, GL_COMPILE_STATUS, &status);

        // Displaying error message
        if (!status) {
                char error[1024];
                glGetShaderInfoLog(shader_module, 1024, NULL, error);
                fprintf(stderr, "%s\n", error);
        }

        free((void*)source);
        return shader_module;
}

unsigned int make_shader(const char* vertex_filepath,
                         const char* fragment_filepath) {
        unsigned int vertex_module =
            make_module(vertex_filepath, GL_VERTEX_SHADER);
        unsigned int fragment_module =
            make_module(fragment_filepath, GL_FRAGMENT_SHADER);

	// Create the shader program with both our modules
        unsigned int shader = glCreateProgram();
	glAttachShader(shader, vertex_module);
	glAttachShader(shader, fragment_module);

	// Link the program to our main program
	glLinkProgram(shader);

	int status;
	glGetProgramiv(shader, GL_LINK_STATUS, &status);
	if (!status) {
		char error[1024];
		glGetProgramInfoLog(shader, 1024, NULL, error);
		fprintf(stderr, "%s\n", error);
	}
        
        glUseProgram(shader);
        
	glDeleteShader(vertex_module);
	glDeleteShader(fragment_module);

	// Done !
	return shader;
}
