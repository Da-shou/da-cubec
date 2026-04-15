#ifndef CHUNK_STORE_H
#define CHUNK_STORE_H

#include <stdbool.h>
#include <stdint.h>
#include "chunk.h"

/* Power-of-2 capacity. Handles up to ~768 modified chunks at 0.75 load. */
#define CHUNK_STORE_CAPACITY 1024

typedef struct {
    int cx, cz;
    bool occupied;
    uint8_t* blocks; /* heap-allocated [CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ] */
} chunk_store_entry_t;

typedef struct {
    chunk_store_entry_t entries[CHUNK_STORE_CAPACITY];
} chunk_store_t;

void chunk_store_init(chunk_store_t* store);
void chunk_store_destroy(chunk_store_t* store);

/** Save a full blocks snapshot for (cx, cz). Overwrites any existing entry. */
void chunk_store_save(chunk_store_t* store, int cx, int cz,
                      const uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]);

/** Copy saved blocks into out_blocks. Returns true if an entry was found. */
bool chunk_store_load(chunk_store_t* store, int cx, int cz,
                      uint8_t out_blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]);

#endif /* CHUNK_STORE_H */
