/**
 * @file blocks.h
 * @authors Da-shou
 * @brief Header file defining the ID list of each block type as well as the struct
 * used to describe a UV coordinate for a block. Each block has 6 sets of uv
 * coordinates, for each face of the block. Each set is two coordinates floats between
 * 0 and 1. All block texture come from a single texture atlas.
 */

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdbool.h>
#include <cglm/types.h>

static const float tile_offset = 0.25F;

/**
 * @brief Stores two texture coordinates. */
typedef struct {
    float u; /**< Horizontal texture coordinate [0.0F-1.0F] */
    float v; /**< Vertical texture coordinate [0.0F-1.0F] */
} uv_t;

/**
 * @brief Stores the top-left coordinates of a block UV on the texture
 * Atlas.*/
typedef struct {
    uv_t top;    /**< UV mapping of the top of the block */
    uv_t bottom; /**< UV mapping of the bottom of the block */
    uv_t front;  /**< UV mapping of the front of the block */
    uv_t back;   /**< UV mapping of the back of the block */
    uv_t left;   /**< UV mapping of the left of the block */
    uv_t right;  /**< UV mapping of the right of the block */
} block_uv_t;

/**
 * @brief Assigns an id to each block type.*/
typedef enum {
    BLOCK_AIR         = 0, /**< Empty block, cannot be broken.*/
    BLOCK_STONE       = 1,
    BLOCK_COBBLESTONE = 2,
    BLOCK_DIRT        = 3,
    BLOCK_GRASS       = 4,
    BLOCK_SAND        = 5,
    BLOCK_GLOW        = 6
} block_type_t;

/**
 * @brief Defines the texture mapping for each block type */
static const block_uv_t block_uvs[] = {
    [BLOCK_STONE] =
        {
            .top    = {0.00F, 0.00F},
            .bottom = {0.00F, 0.00F},
            .front  = {0.00F, 0.00F},
            .back   = {0.00F, 0.00F},
            .left   = {0.00F, 0.00F},
            .right  = {0.00F, 0.00F},
        },
    [BLOCK_COBBLESTONE] =
        {
            .top    = {0.25F, 0.00F},
            .bottom = {0.25F, 0.00F},
            .front  = {0.25F, 0.00F},
            .back   = {0.25F, 0.00F},
            .left   = {0.25F, 0.00F},
            .right  = {0.25F, 0.00F},
        },
    [BLOCK_DIRT] =
        {
            .top    = {0.5F, 0.00F},
            .bottom = {0.5F, 0.00F},
            .front  = {0.5F, 0.00F},
            .back   = {0.5F, 0.00F},
            .left   = {0.5F, 0.00F},
            .right  = {0.5F, 0.00F},
        },
    [BLOCK_GRASS] =
        {
            .top    = {0.00F, 0.25F},
            .bottom = {0.5F, 0.00F},
            .front  = {0.75F, 0.00F},
            .back   = {0.75F, 0.00F},
            .right  = {0.75F, 0.00F},
            .left   = {0.75F, 0.00F},
        },
    [BLOCK_SAND] =
        {
            .top    = {0.25F, 0.25F},
            .bottom = {0.25F, 0.25F},
            .front  = {0.25F, 0.25F},
            .back   = {0.25F, 0.25F},
            .right  = {0.25F, 0.25F},
            .left   = {0.25F, 0.25F},
        },
    [BLOCK_GLOW] =
        {
            .top    = {0.5F, 0.25F},
            .bottom = {0.5F, 0.25F},
            .front  = {0.5F, 0.25F},
            .back   = {0.5F, 0.25F},
            .right  = {0.5F, 0.25F},
            .left   = {0.5F, 0.25F},
        },
};

static const block_type_t light_blocks[] = {
    BLOCK_GLOW,
};

/**
 * @brief Checks if the given block type from block_type_t should emit light.
 * @param block_type The type of block to check
 * @return true if the block should emit light, false if not.
 */
bool is_light_block(block_type_t block_type);

#endif /* BLOCKS_H */
