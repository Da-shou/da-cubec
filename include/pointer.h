#ifndef POINTER_H
#define POINTER_H

#include <cglm/cglm.h>

#include <camera.h>
#include <chunk.h>
#include <blocks.h>

typedef enum { X, Y, Z } axis_t;

/**
 * @brief Returns the type of block that has been scanned using a raycast
 * in the chunk array. Uses the Fast Voxel Traversal Algorithm.
 * @param chunk Pointer to the chunk that is getting analzed
 * @param camera Pointer to the camera that the raycast will start from
 * @param max_distance Max length of the raycast
 * @param pointed_block Pointer to a vec3 that will be filled with the
 * coordinates
 * @param neighbour_block Pointer to a vec3 that will be filled with the
 * virtual coordinate of the block that could be place. of the block. */
block_type_t get_pointed_block(chunk_t* chunk, camera_t* camera,
                               float max_distance, vec3* pointed_block,
                               vec3* neighbour_block);

/**
 * @brief Processes the block pointed to if it's not air.
 * @param block_position vec3 containing the position of the block pointed
 * at.
 * @param last Last axis hit by the raycast. Allows us to know which face
 * of the block is being pointed at.
 * @param step_x, step_y, step_z Distances to travel to get to the
 * neighbour block.*/
void process_block(vec3 block_position, axis_t last, vec3* pointed_block,
                   vec3* neighbour_block, int step_x, int step_y,
                   int step_z);
#endif
