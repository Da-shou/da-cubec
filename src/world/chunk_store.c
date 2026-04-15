#include <stdlib.h>
#include <string.h>

#include "world/chunk_store.h"

#define BLOCKS_SIZE (CHUNK_SIZE_XZ * CHUNK_SIZE_Y * CHUNK_SIZE_XZ)

/**
 * Creates a basic hash key to store the chunk in a 1024 hashmap.
 * @param chunk_x X Coordinate of the chunk
 * @param chunk_z Z Coordinate of the chunk
 * @return Hash key for the chunk coordinates
 */
static int hash_key(const int chunk_x, const int chunk_z) {
    const uint32_t entity_key =
        ( (uint32_t)chunk_x * 2654435761U ) ^ ( (uint32_t)chunk_z * 2246822519U );
    return (int)(entity_key & (CHUNK_STORE_CAPACITY - 1));
}

void chunk_store_init(chunk_store_t* store) {
    memset(store->entries, 0, sizeof(store->entries));
}

void chunk_store_destroy(const chunk_store_t* store) {
    for (int i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        if (store->entries[i].occupied) { free(store->entries[i].blocks); }
    }
}

void chunk_store_save(
    chunk_store_t* store, const int chunk_x, const int chunk_z,
    const uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]) {
    /* Getting the hashkey for the current chunk */
    int key = hash_key(chunk_x, chunk_z);

    for (int16_t i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        /* Retriving the entry in the hasmap */
        chunk_store_entry_t* entry = &store->entries[key];
        if (!entry->occupied || (entry->cx == chunk_x && entry->cz == chunk_z)) {
            /* If the space is not occupied, allocate the space and
             * save the entire chunk into memory. */
            if (!entry->occupied) {
                entry->blocks = malloc((size_t)BLOCKS_SIZE);
                entry->occupied = true;
                entry->cx = chunk_x;
                entry->cz = chunk_z;
            }
            /* Copy all blocks from blocks to the hash map entry. */
            memcpy(entry->blocks, blocks, (size_t)BLOCKS_SIZE);
            return;
        }
        /* In case of collision, alternate the key. */
        key = (key + 1) & (CHUNK_STORE_CAPACITY - 1);
    }
}

bool chunk_store_load(
    const chunk_store_t* store, int const chunk_x, int const chunk_z,
    uint8_t out_blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]) {
    /* Getting the hashkey for the current chunk */
    int key = hash_key(chunk_x, chunk_z);
    for (int16_t i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        /* Getting the entry from the hashmap */
        const chunk_store_entry_t* entry = &store->entries[key];
        /* Nothing in here, return */
        if (!entry->occupied) { return false; }
        /* If the chunk coordinate coorespond, copy the data into
         * out_blocks. */
        if (entry->cx == chunk_x && entry->cz == chunk_z) {
            memcpy(out_blocks, entry->blocks, (size_t)BLOCKS_SIZE);
            return true;
        }
        /* In case of collision, alternate the key. */
        key = (key + 1) & (CHUNK_STORE_CAPACITY - 1);
    }
    return false;
}
