#ifndef CUBE_H
#define CUBE_H

#include <mesh.h>
#include <cglm/cglm.h>
#include <shader.h>
#include <material.h>

/**
 * @brief Helper struct for building cubes quickly
 * @param mesh Contains the model's informations
 * @param position Coordinates of the cube
 * @param rotation Rotation factor
 * @param scale Scaling factor */
typedef struct {
        mesh_t* mesh;
       	float* texture_coordinates; 
	material_t* material;
	vec3 position;
        vec3 rotation;
        vec3 scale;
        mat4 model;
} cube_t;

/**
 * @brief Initialize the cube to make it ready to draw on screen 
 * @param c Pointer to the cube struct to initalize. 
 * @param m Pointer to the material to bind when the cube is drawn. */
void cube_init(cube_t* c, material_t* m);

/** @brief Updates the cube's position, scale and rotation based on its
 * attributes. */
void cube_update(cube_t* c);

/** @brief Calls the necessary functions to draw the cube, mainly binding
 * the VAO, drawing, and unbinding the VAO. */
void cube_draw(cube_t* c, shader_t* s);

/** @brief Frees the static mesh's VAO, VBOs and EAO if this is the last
 * cube to be freed. */
void cube_destroy(cube_t* c);

#endif
