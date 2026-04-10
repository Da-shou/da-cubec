#ifndef POINTER_H
#define POINTER_H

#include <cglm/cglm.h>

#include <camera.h>
#include <chunk.h>
#include <blocks.h>

/**
 * @brief Returns the type of block that has been scanned using a raycast
 * in the chunk array. Uses the Fast Voxel Traversal Algorithm.
 * @param chunk Pointer to the chunk that is getting analzed
 * @param camera Pointer to the camera that the raycast will start from
 * @param max_distance Max length of the raycast
 * @param vec3 Pointer to a vec3 that will be filled with the coordinates
 * of the block. */
block_type_t get_pointed_block(chunk_t* chunk, camera_t* camera,
                               float max_distance, vec3* block);

#endif
