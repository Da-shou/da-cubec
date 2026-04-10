#ifndef BLOCKS_H
#define BLOCKS_H

static const float TILE_OFFSET = 0.25f;

/**
 * @brief Stores two texture coordinates. */
typedef struct {
	float u, v;
} uv_t;

/**
 * @brief Stores the top-left coordinates of a block UV on the texture Atlas.*/
typedef struct {
        uv_t top;
        uv_t bottom;
        uv_t front;
        uv_t back;
        uv_t left;
        uv_t right;
} block_uv_t;

/**
 * @brief Assigns an id to each block type.*/
typedef enum {
        BLOCK_AIR = 0,
        BLOCK_STONE = 1,
        BLOCK_COBBLESTONE = 2,
        BLOCK_DIRT = 3,
	BLOCK_GRASS = 4,
        BLOCK_SAND = 5
} block_type_t;

/**
 * @brief Defines the texture mapping for each block type */
static const block_uv_t BLOCK_UVS[] = {
    [BLOCK_STONE] = {
	    .top = {0.00f, 0.00f},
	    .bottom = {0.00f, 0.00f},
	    .front = {0.00f, 0.00f},
	    .back = {0.00f, 0.00f},
	    .left = {0.00f, 0.00f},
	    .right = {0.00f, 0.00f},
    },
    [BLOCK_COBBLESTONE] = {
	    .top = {0.25f, 0.00f},
	    .bottom = {0.25f, 0.00f},
	    .front = {0.25f, 0.00f},
	    .back = {0.25f, 0.00f},
	    .left = {0.25f, 0.00f},
	    .right = {0.25, 0.00f},
    },
    [BLOCK_DIRT] = {
	    .top = {0.5f, 0.00f},
	    .bottom = {0.5f, 0.00f},
	    .front = {0.5f, 0.00f},
	    .back = {0.5f, 0.00f},
	    .left = {0.5f, 0.00f},
	    .right = {0.5f, 0.00f},
    },
    [BLOCK_GRASS] = {
	    .top = {0.00f, 0.25f},
	    .bottom = {0.5f, 0.00f},
	    .front = {0.75f, 0.00f},
	    .back = {0.75f, 0.00f},
	    .right = {0.75f, 0.00f},
	    .left = {0.75f, 0.00f},
    },
    [BLOCK_SAND] = {
	    .top = {0.25f, 0.25f},
	    .bottom = {0.25f, 0.25f},
	    .front = {0.25f, 0.25f},
	    .back = {0.25f, 0.25f},
	    .right = {0.25f, 0.25f},
	    .left = {0.25f, 0.25f},
    },
};

#endif
