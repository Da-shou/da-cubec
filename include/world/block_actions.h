
#ifndef BLOCK_ACTIONS_H
#define BLOCK_ACTIONS_H

#include <stdint.h>

#include "world/world.h"
#include "world/blocks.h"
#include "player.h"

/**
 * @brief Places a block in the world at the give position.
 * @param position World-coordinates of the block that should be placed.
 * @param type Type of the block being placed from block_type_t.
 * @return 0 if world was rebuilt correclty, -1 if world building failed.
 */
int8_t place_block(world_t* world, const player_t* player, vec3 position, block_type_t block_type);

/**
 * @brief Breaks a block in the world at the given position.
 * @param position World-coordinates of the block that should be broken.
 * @return 0 if world was rebuilt correclty, -1 if world building failed.
 */
int8_t break_block(world_t* world, vec3 position);

#endif /* BLOCK_ACTIONS_H */
