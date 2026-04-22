/**
 * @file chunk_store.h
 * Header file for storage struct and functions. They are used to store
 * modified chunks somewhere in memory and each chunk that is going to be loaded is
 * checked in this storage if its hash exists, and if it does, its blocks will be pulled
 * from this storage instead of being regenerated following the world generator logic.
 * This allows for the player to place blocks, step away from their construction until
 * it unloads, then come back and still see their construction.
 * @authors {Da-shou}
 */

#ifndef CHUNK_STORE_H
#define CHUNK_STORE_H

#define GLFW_INCLUDE_NONE
#include <glad/gl.h>

#include <stdbool.h>
#include <stdint.h>

#include "chunk.h"

/* Power-of-2 capacity. Handles up to ~768 modified chunks at 0.75 load. */
#define CHUNK_STORE_CAPACITY 1024

/**
 * @brief Represents an entry in the chunk store hashmap.
 */
typedef struct {
    int cx;          /**< X coordinate of the chunk in chunk coordinates. */
    int cz;          /**< Z coordinate of the chunk in chunk coordinates. */
    bool occupied;   /**< If true, signals that an entry has that hash and that the entry
      colliding with that hash must use another hash. */
    uint8_t* blocks; /**< ID blocks array to keep in storage if the chunks needs to be
    redrawn. */
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
void chunk_store_save(chunk_store_t* store, int chunk_x, int chunk_z,
                      const uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]);

/** @brief Copy saved blocks into out_blocks. Returns true if an entry was
 * found.
 * @param store Pointer to chunk hashmap to look into
 * @param chunk_x X Coordinate of the chunk
 * @param chunk_z Z Coordinate of the chunk
 * @param out_blocks The blocks from the hashmap entry will be copied
 * into this buffer.
 */
bool chunk_store_load(const chunk_store_t* store, int chunk_x, int chunk_z,
                      uint8_t out_blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]);

#endif /* CHUNK_STORE_H */
