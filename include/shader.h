#ifndef SHADER_H
#define SHADER_H

/** 
 * @brief Structure to represent an OpenGL shader 
 * @attribute */
typedef struct {
        unsigned int ID;
} shader;

/**
 * @brief Initalizes a shader using two shader files.
 * @param s Shader to initalize.
 * @param vertex_filepath Path to vertex source file.
 * @param fragment_filepath Path to fragment source file. */
void shader_init(shader* s, const char* vertex_filepath,
                 const char* fragment_filepath);

/**
 * @brief Makes this shader in use by the program.
 * @param s Shader to make the program use. */
void shader_use(shader* s);

/**
 * @brief Frees the shader program at s from memory.
 * @param s Shader to destroy. */
void shader_destroy(shader* s);

/**
 * @brief Sets a new boolean uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_bool(shader* s, const char* name, _Bool value);

/**
 * @brief Sets a new integer uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_int(shader* s, const char* name, int value);

/**
 * @brief Sets a new floating point uniform variable.
 * @param s Shader to modify.
 * @param name Name of uniform variable to set.
 * @param value Value to set the uniform variable to. */
void shader_set_float(shader* s, const char* name, float value);

#endif // SHADER_H
