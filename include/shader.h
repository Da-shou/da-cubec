/**
 * @file shader.h
 * @authors Da-shou
 * Header file declaring a shader structure containing all the elements needed to
 * create an OpenGL shader program, as well as the functions signatures to do so.
 */

#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <cglm/types.h>

/**
 * @brief Structure to represent an OpenGL shader
 * @param ID unsigned integer locating the shader. */
typedef struct {
    GLuint id; /**< OpenGL ID of the created shader program */
} shader_t;

/**
 * @brief Initalizes a shader using two shader files.
 * @param shader Shader to initalize.
 * @param vertex_filepath Path to vertex source file.
 * @param fragment_filepath Path to fragment source file. */
void shader_init(shader_t* shader, const char* vertex_filepath,
                 const char* fragment_filepath);

/**
 * @brief Makes this shader in use by the program.
 * @param shader Shader to make the program use. */
void shader_use(const shader_t* shader);

/**
 * @brief Frees the shader program at shader from memory.
 * @param shader Shader to destroy. */
void shader_destroy(const shader_t* shader);

/**
 * @brief Sets a new boolean uniform variable.
 * @param shader Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_bool(const shader_t* shader, const char* name, bool value);

/**
 * @brief Sets a new integer uniform variable.
 * @param shader Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_int(const shader_t* shader, const char* name, int value);

/**
 * @brief Sets a new floating point uniform variable.
 * @param shader Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_float(const shader_t* shader, const char* name, float value);

/**
 * @brief Sets a new 4x4 matrix uniform variable.
 * @param shader Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_mat4(const shader_t* shader, const char* name, mat4 value);

/**
 * @brief Sets a new vec3 uniform variable.
 * @param shader Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_vec3(const shader_t* shader, const char* name, vec3 value);

#endif // SHADER_H
