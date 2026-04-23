/**
 * @file material.h
 * @authors Da-shou
 * Header file declaring the material structure containing all elements needed to make
 * an OpenGL texture and stores its OpenGL ID.
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "gl_glfw_include.h"

/**
 * @brief Structure storing the important info about a material. For now,
 * this is simply a texture. */
typedef struct {
    GLuint texture; /**< OpenGL ID of the texture created */
} material_t;

/**
 * @brief Creates an OpenGL texture using an image and assigns its id to
 * the material attribute.
 * @param material Pointer to the material which will be modified.
 * @param filename Path to the texture file.*/
void material_create(material_t* material, const char* filename);

/**
 * @brief Makes the necessary calls so that the material is used.
 * @param material Pointer to the material which will be used.
 * @param unit Number of the unit of the texture.*/
void material_use(const material_t* material, int unit);

/**
 * @brief Destroys the allocation OpenGL material.
 * @param material Pointer to the material to be destroyed. */
void material_destroy(const material_t* material);

#endif
