#ifndef BLOCKS_H
#define BLOCKS_H

static const float TILE_OFFSET = 0.25f;

/**
 * @brief Stores the top-left coordinates of a block UV on the texture Atlas.*/
typedef struct {
        float u, v;
} block_uv_t;

/**
 * @brief Assigns an id to each block type.*/
typedef enum {
        BLOCK_AIR = 0,
        BLOCK_STONE = 1,
        BLOCK_COBBLESTONE = 2,
        BLOCK_DIRT = 3,
        BLOCK_SAND = 4
} block_type_t;

static const block_uv_t BLOCK_UVS[] = {
    [BLOCK_STONE] = {0.00f, 0.00f},
    [BLOCK_COBBLESTONE] = {0.25f, 0.00f},
    [BLOCK_DIRT] = {0.5f, 0.00f},
    [BLOCK_SAND] = {0.75f, 0.00f},
};

#endif
