#ifndef CHUNK_STORE_H
#define CHUNK_STORE_H

#include <stdbool.h>
#include <stdint.h>

#include "chunk.h"

/* Power-of-2 capacity. Handles up to ~768 modified chunks at 0.75 load. */
#define CHUNK_STORE_CAPACITY 1024

/**
 * @brief Represents an entry in the chunk store hashmap.
 */
typedef struct {
    int cx, cz;
    bool occupied;
    uint8_t* blocks;
} chunk_store_entry_t;

/**
 * @brief Represents a chunk storage hashmasp with a fixed capacity. */
typedef struct {
    chunk_store_entry_t entries[CHUNK_STORE_CAPACITY];
} chunk_store_t;

/**
 * @brief Initializes a chunk_store_t struct.
 * @param store Pointer to the store to initialize. */
void chunk_store_init(chunk_store_t* store);

/**
 * @brief Destroys the content of a chunk hashmap.
 * @param store Pointer to the store to destroy. */
void chunk_store_destroy(const chunk_store_t* store);

/** @brief Save a full blocks snapshot for (cx, cz). Overwrites any
 * existing entry.
 * @param store Hashmap to look into
 * @param chunk_x X Coordinate of the chunk
 * @param chunk_z Z Coordinate of the chunk
 * @param blocks The blocks to save into the hashmap entry.
 */
void chunk_store_save(
    chunk_store_t* store, int chunk_x, int chunk_z,
    const uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]);

/** @brief Copy saved blocks into out_blocks. Returns true if an entry was
 * found.
 * @param store Pointer to chunk hashmap to look into
 * @param chunk_x X Coordinate of the chunk
 * @param chunk_z Z Coordinate of the chunk
 * @param out_blocks The blocks from the hashmap entry will be copied
 * into this buffer.
 */
bool chunk_store_load(
    const chunk_store_t* store, int chunk_x, int chunk_z,
    uint8_t out_blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]);

#endif /* CHUNK_STORE_H */
