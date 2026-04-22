#ifndef POINTER_H
#define POINTER_H

#include "game_state.h"

typedef enum { X, Y, Z } axis_t;

/**
 * @brief Returns the type of block that has been scanned using a raycast
 * in the chunk array. Uses the Fast Voxel Traversal Algorithm.
 * @param game_state Pointer to current game_state
 * @param max_distance How far the pointed block can be
 * @returns ID of the block from the block_type_t enum. */
uint8_t get_pointed_block(game_state_t* game_state, float max_distance);

/**
 * @brief Takes XYZ coordinates of a block through block_position and an
 * axis and finds its neighbour following the given axis.
 * @param block_position vec3 containing the position of the block pointed
 * at.
 * @param last Last axis hit by the ray-cast. Allows us to know which face
 * of the block is being pointed at.
 * @param pointed_block The pointer to the vector that will be filled with
 * the coordinates of the currently targeted block.
 * @param neighbour_block The pointer to the vector that will be filled
 * with the coordinates of the neighbouring block once found.
 * @param step_x, step_y, step_z Distances to travel to get to the
 * neighbour block.
 */
void process_block(vec3 block_position, axis_t last, vec3* pointed_block,
                   vec3* neighbour_block, int step_x, int step_y, int step_z);
#endif
