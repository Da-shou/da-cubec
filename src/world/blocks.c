/**
 * @file blocks.c
 * @authors Da-shou
 * @brief Implementation of block-related functions for the game world.
 */

#include "world/blocks.h"
#include <stdbool.h>
#include <stddef.h>
#include <cglm/cglm.h>

bool is_light_block(const block_type_t block_type) {
    for (size_t i = 0; i < sizeof(light_blocks) / sizeof(block_type_t); ++i) {
        if (block_type == light_blocks[i]) { return true; }
    }
    return false;
}
