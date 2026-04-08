#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <cglm/types.h>

/**
 * @brief Structure to represent an OpenGL shader
 * @param ID unsigned integer locating the shader. */
typedef struct {
        unsigned int id;
} shader_t;

/**
 * @brief Initalizes a shader using two shader files.
 * @param s Shader to initalize.
 * @param vertex_filepath Path to vertex source file.
 * @param fragment_filepath Path to fragment source file. */
void shader_init(shader_t* s, const char* vertex_filepath,
                 const char* fragment_filepath);

/**
 * @brief Makes this shader in use by the program.
 * @param s Shader to make the program use. */
void shader_use(shader_t* s);

/**
 * @brief Frees the shader program at s from memory.
 * @param s Shader to destroy. */
void shader_destroy(shader_t* s);

/**
 * @brief Sets a new boolean uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_bool(shader_t* s, const char* name, bool value);

/**
 * @brief Sets a new integer uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_int(shader_t* s, const char* name, int value);

/**
 * @brief Sets a new floating point uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_float(shader_t* s, const char* name, float value);

/**
 * @brief Sets a new 4x4 matrix uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_mat4(shader_t* s, const char* name, mat4 value);

#endif // SHADER_H
