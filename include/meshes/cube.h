#ifndef CUBE_H
#define CUBE_H

#include <mesh.h>
#include <cglm/cglm.h>

/** 
 * @brief Helper struct for building cubes quickly
 * @param mesh Contains the model's informations 
 * @param position Coordinates of the cube 
 * @param rotation Rotation factor 
 * @param scale Scaling factor */
typedef struct {
	mesh_t* mesh;	
	vec3 position;
	vec3 rotation;
	vec3 scale;
} cube_t;

/**
 * @brief Initialize the cube to make it ready to draw on screen */
void cube_init(cube_t* c);
void cube_draw(cube_t* c);
void cube_free(cube_t* c);

#endif
