#include <stdlib.h>
#include <string.h>

#include "world/chunk_store.h"

#define BLOCKS_SIZE (CHUNK_SIZE_XZ * CHUNK_SIZE_Y * CHUNK_SIZE_XZ)

/**
 * Creates a basic hash key to store the chunk in a 1024 hashmap.
 * @param cx X Coordinate of the chunk
 * @param cz Z Coordinate of the chunk
 * @return Hash key for the chunk coordinates
 */
static int hash_key(const int cx, const int cz) {
    const uint32_t h =
        (uint32_t)cx * 2654435761u ^ (uint32_t)cz * 2246822519u;
    return (int)(h & (CHUNK_STORE_CAPACITY - 1));
}

void chunk_store_init(chunk_store_t* store) {
    memset(store->entries, 0, sizeof(store->entries));
}

void chunk_store_destroy(const chunk_store_t* store) {
    for (int i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        if (store->entries[i].occupied) free(store->entries[i].blocks);
    }
}

void chunk_store_save(
    chunk_store_t* store, const int cx, const int cz,
    const uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]) {
    /* Getting the hashkey for the current chunk */
    int key = hash_key(cx, cz);

    for (int16_t i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        /* Retriving the entry in the hasmap */
        chunk_store_entry_t* e = &store->entries[key];
        if (!e->occupied || (e->cx == cx && e->cz == cz)) {
            /* If the space is not occupied, allocate the space and
             * save the entire chunk into memory. */
            if (!e->occupied) {
                e->blocks = malloc(BLOCKS_SIZE);
                e->occupied = true;
                e->cx = cx;
                e->cz = cz;
            }
            /* Copy all blocks from blocks to the hash map entry. */
            memcpy(e->blocks, blocks, BLOCKS_SIZE);
            return;
        }
        /* In case of collision, alternate the key. */
        key = (key + 1) & (CHUNK_STORE_CAPACITY - 1);
    }
}

bool chunk_store_load(
    const chunk_store_t* store, int const cx, int const cz,
    uint8_t out_blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]) {
    /* Getting the hashkey for the current chunk */
    int key = hash_key(cx, cz);
    for (int16_t i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        /* Getting the entry from the hashmap */
        const chunk_store_entry_t* e = &store->entries[key];
        /* Nothing in here, return */
        if (!e->occupied) return false;
        /* If the chunk coordinate coorespond, copy the data into
         * out_blocks. */
        if (e->cx == cx && e->cz == cz) {
            memcpy(out_blocks, e->blocks, BLOCKS_SIZE);
            return true;
        }
        /* In case of collision, alternate the key. */
        key = (key + 1) & (CHUNK_STORE_CAPACITY - 1);
    }
    return false;
}
