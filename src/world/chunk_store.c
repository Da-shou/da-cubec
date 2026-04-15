#include <stdlib.h>
#include <string.h>

#include "world/chunk_store.h"

#define BLOCKS_SIZE (CHUNK_SIZE_XZ * CHUNK_SIZE_Y * CHUNK_SIZE_XZ)

static int hash_key(int cx, int cz) {
    uint32_t h = (uint32_t)cx * 2654435761u ^ (uint32_t)cz * 2246822519u;
    return (int)(h & (CHUNK_STORE_CAPACITY - 1));
}

void chunk_store_init(chunk_store_t* store) {
    memset(store->entries, 0, sizeof(store->entries));
}

void chunk_store_destroy(chunk_store_t* store) {
    for (int i = 0; i < CHUNK_STORE_CAPACITY; i++) {
        if (store->entries[i].occupied)
            free(store->entries[i].blocks);
    }
}

void chunk_store_save(chunk_store_t* store, int cx, int cz,
                      const uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]) {
    int i = hash_key(cx, cz);
    for (int probe = 0; probe < CHUNK_STORE_CAPACITY; probe++) {
        chunk_store_entry_t* e = &store->entries[i];
        if (!e->occupied || (e->cx == cx && e->cz == cz)) {
            if (!e->occupied) {
                e->blocks = malloc(BLOCKS_SIZE);
                e->occupied = true;
                e->cx = cx;
                e->cz = cz;
            }
            memcpy(e->blocks, blocks, BLOCKS_SIZE);
            return;
        }
        i = (i + 1) & (CHUNK_STORE_CAPACITY - 1);
    }
    /* Table is full — silently drop. Extremely unlikely at CAPACITY=1024. */
}

bool chunk_store_load(chunk_store_t* store, int cx, int cz,
                      uint8_t out_blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]) {
    int i = hash_key(cx, cz);
    for (int probe = 0; probe < CHUNK_STORE_CAPACITY; probe++) {
        chunk_store_entry_t* e = &store->entries[i];
        if (!e->occupied)
            return false;
        if (e->cx == cx && e->cz == cz) {
            memcpy(out_blocks, e->blocks, BLOCKS_SIZE);
            return true;
        }
        i = (i + 1) & (CHUNK_STORE_CAPACITY - 1);
    }
    return false;
}
