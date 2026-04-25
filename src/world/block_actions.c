/**
 * @file block_actions.c
 * @authors Da-shou
 * @brief Implementation of block-related functions for interacting with the blocks of the
 * game world.
 */

#include "world/blocks.h"

#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <cglm/cglm.h>

#include "world/world.h"
#include "player.h"

int8_t place_block(world_t* world, const player_t* player, float* position,
                   block_type_t block_type) {
    if (!world_valid_position(world, position)) { return 0; }

    const int chunk_x   = (int)floorf(position[0] / (float)CHUNK_SIZE_XZ);
    const int chunk_z   = (int)floorf(position[2] / (float)CHUNK_SIZE_XZ);
    const int n_local_x = (int)floorf(position[0]) - (chunk_x * CHUNK_SIZE_XZ);
    const int n_local_y = (int)floorf(position[1]);
    const int n_local_z = (int)floorf(position[2]) - (chunk_z * CHUNK_SIZE_XZ);

    /* Player AABB */
    const float player_half_width = player->width / 2.0F;
    vec3 player_aabb[2] = {{player->position[0] - player_half_width, player->position[1],
                            player->position[2] - player_half_width},
                           {player->position[0] + player_half_width,
                            player->position[1] + player->height,
                            player->position[2] + player_half_width}};

    /* Block AABB */
    vec3 block_aabb[2] = {{position[0], position[1], position[2]},
                          {position[0] + 1.0F, position[1] + 1.0F, position[2] + 1.0F}};

    /* If player and future placed block interesect, do not place the block. */
    const bool overlap = glm_aabb_aabb(player_aabb, block_aabb);

    if (overlap) { return 0; }

    chunk_t* chunk = world_get_chunk(world, chunk_x, chunk_z);

    /* Can only place a block if there is air at the wanted spot */
    if (chunk->blocks[n_local_x][n_local_y][n_local_z] != (uint8_t)BLOCK_AIR) {
        return 0;
    }

    chunk->blocks[n_local_x][n_local_y][n_local_z] = (uint8_t)block_type;

    chunk->modified = true;
    return (int8_t)world_rebuild_after_change(world, chunk_x, chunk_z);
}

int8_t break_block(world_t* world, float* position) {
    const int chunk_x = (int)floorf(position[0] / (float)CHUNK_SIZE_XZ);
    const int chunk_z = (int)floorf(position[2] / (float)CHUNK_SIZE_XZ);
    const int local_x = (int)floorf(position[0]) - (chunk_x * CHUNK_SIZE_XZ);
    const int local_y = (int)floorf(position[1]);
    const int local_z = (int)floorf(position[2]) - (chunk_z * CHUNK_SIZE_XZ);

    chunk_t* chunk                           = world_get_chunk(world, chunk_x, chunk_z);
    chunk->blocks[local_x][local_y][local_z] = (uint8_t)BLOCK_AIR;

    chunk->modified = true;
    return (int8_t)world_rebuild_after_change(world, chunk_x, chunk_z);
}
