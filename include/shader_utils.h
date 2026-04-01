#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

/**
 * @brief Compiles a shader given a filepath and module_type
 * @param filepath Path to the shader file
 * @param module_type Type of shader to compile
 **/
unsigned int make_module(const char* filepath, unsigned int module_type);

/**
 * @brief Compiles a shader program using a vertex and fragment shader.
 * @param vertex_filepath Path to the shader file
 * @param fragment_filepath Type of shader to compile
 **/
unsigned int make_shader(const char* vertex_filepath,
                         const char* fragment_filepath);

#endif // SHADER_UTILS_H
