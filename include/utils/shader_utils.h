#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include "gl_glfw_include.h"

/**
 * @brief Compiles a shader given a filepath and module_type
 * @param filepath Path to the shader file
 * @param module_type Type of shader to compile
 * @returns ID of created module. 0 if creation failed.
 **/
GLuint make_module(const char* filepath, unsigned int module_type);

/**
 * @brief Compiles a shader program using a vertex and fragment shader.
 * @param vertex_filepath Path to the shader file
 * @param fragment_filepath Type of shader to compile
 * @returns ID of shader program created. 0 if compilation failed.
 **/
GLuint make_shader(const char* vertex_filepath, const char* fragment_filepath);

#endif // SHADER_UTILS_H
