#include <cglm/cglm.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "blocks.h"
#include "world.h"

void world_init(world_t* world) {
    const int N = LOADED_CHUNKS_SIZE;
    vec3 dummy = {0.0f, 0.0f, 0.0f};
    for (int sx = 0; sx < N; sx++) {
        for (int sz = 0; sz < N; sz++) {
            chunk_init(&world->chunks[sx][sz], dummy);
            world->slot_cx[sx][sz] = INT_MIN;
            world->slot_cz[sx][sz] = INT_MIN;
        }
    }
    world->last_player_cx = INT_MIN;
    world->last_player_cz = INT_MIN;
    world->generate = NULL;
    world->generator_data = NULL;
}

chunk_t* world_get_chunk(world_t* world, const int cx, const int cz) {
    const int N = LOADED_CHUNKS_SIZE;
    const int sx = ((cx % N) + N) % N;
    const int sz = ((cz % N) + N) % N;
    if (world->slot_cx[sx][sz] == cx && world->slot_cz[sx][sz] == cz)
        return &world->chunks[sx][sz];
    return NULL;
}

void world_update(world_t* world, const vec3 player_pos) {
    const int R = RENDER_DISTANCE;
    const int N = LOADED_CHUNKS_SIZE;

    /* Getting chunk coordinates in which the player is right now. */
    const int pcx = (int)floorf(player_pos[0] / (float)CHUNK_SIZE_XZ);
    const int pcz = (int)floorf(player_pos[2] / (float)CHUNK_SIZE_XZ);

    /* If the player has not crossed a new chunk, do not do anything. */
    if (pcx == world->last_player_cx && pcz == world->last_player_cz)
        return;

    world->last_player_cx = pcx;
    world->last_player_cz = pcz;

    /* dirty[sx][sz] = true if this slot was reloaded this update.
     * Used to avoid rebuilding meshes that did not change. */
    bool dirty[LOADED_CHUNKS_SIZE][LOADED_CHUNKS_SIZE] = {0};

    /* Pass 1: terrain — evict stale slots, fill new ones. */
    for (int sx = 0; sx < N; sx++) {
        for (int sz = 0; sz < N; sz++) {
            /* base_cx is the leftmost world chunk, we get this
             * by subtracting the render distance from the player's
             * x chunk coordinates.
             *
             * base_sx is the index of the slot that chunk belongs to.
             * the first % N gets us the index. Then, the + N % N handles
             * negative world coordinates.
             *
             * Adding the current chunk (sx) to the base chunk_index)
             * to get the target chunk's x coordinate.
             * Same logic for the z coordinate.
             */
            const int base_cx   = pcx - R;
            const int base_sx   = ((base_cx % N) + N) % N;
            const int target_cx = base_cx + ((sx - base_sx + N) % N);

            const int base_cz   = pcz - R;
            const int base_sz   = ((base_cz % N) + N) % N;
            const int target_cz = base_cz + ((sz - base_sz + N) % N);

            /* If the target chunk for this slot is already loaded, skip. */
            if (world->slot_cx[sx][sz] == target_cx &&
                world->slot_cz[sx][sz] == target_cz)
                continue;

            /* Clear blocks and reset mesh counters. */
            chunk_t* slot = &world->chunks[sx][sz];
            memset(slot->blocks, 0, sizeof(slot->blocks));
            slot->mesh.vertex_count = 0;
            slot->mesh.index_count  = 0;

            /* Update the world-space position used by chunk_draw. */
            slot->position[0] = (float)(target_cx * CHUNK_SIZE_XZ);
            slot->position[1] = 0.0f;
            slot->position[2] = (float)(target_cz * CHUNK_SIZE_XZ);

            world->slot_cx[sx][sz] = target_cx;
            world->slot_cz[sx][sz] = target_cz;

            if (world->generate)
                world->generate(slot, target_cx, target_cz,
                                world->generator_data);

            dirty[sx][sz] = true;
        }
    }

    /* Only rebuilding the dirty meshes that have just been built. */
    for (int sx = 0; sx < N; sx++) {
        for (int sz = 0; sz < N; sz++) {
            const int wsx = (sx - 1 + N) % N;
            const int esx = (sx + 1) % N;
            const int ssz = (sz - 1 + N) % N;
            const int nsz = (sz + 1) % N;
            if (dirty[sx][sz]  || dirty[wsx][sz] || dirty[esx][sz] ||
                dirty[sx][ssz] || dirty[sx][nsz])
                world_build_chunk(world, sx, sz);
        }
    }
}

void world_build_chunk(world_t* world, const int sx, const int sz) {
    const int cx = world->slot_cx[sx][sz];
    const int cz = world->slot_cz[sx][sz];
    const chunk_neighbours_t neighbors = {
        .west = world_get_chunk(world, cx - 1, cz),
        .east = world_get_chunk(world, cx + 1, cz),
        .south = world_get_chunk(world, cx, cz - 1),
        .north = world_get_chunk(world, cx, cz + 1),
    };
    chunk_t* chunk = world_get_chunk(world, cx, cz);
    chunk_build_mesh(chunk, &chunk->mesh, neighbors);
}

static void rebuild_if_loaded(world_t* world, const int cx, const int cz) {
    const int N = LOADED_CHUNKS_SIZE;
    const int sx = ((cx % N) + N) % N;
    const int sz = ((cz % N) + N) % N;
    if (world->slot_cx[sx][sz] == cx && world->slot_cz[sx][sz] == cz)
        world_build_chunk(world, sx, sz);
}

// clang-format off
void world_rebuild_after_change(world_t* world, const int chunk_x,
                                const int chunk_z, const int local_x,
                                const int local_z) {
    rebuild_if_loaded(world, chunk_x, chunk_z);
    if (local_x == 0)
        rebuild_if_loaded(world, chunk_x - 1, chunk_z);
    if (local_x == CHUNK_SIZE_XZ - 1)
        rebuild_if_loaded(world, chunk_x + 1, chunk_z);
    if (local_z == 0)
        rebuild_if_loaded(world, chunk_x, chunk_z - 1);
    if (local_z == CHUNK_SIZE_XZ - 1)
        rebuild_if_loaded(world, chunk_x, chunk_z + 1);
}

// clang-format on

void world_draw(world_t* world, const shader_t* shader, material_t* atlas,
                vec4 frustum[6]) {
    const int N = LOADED_CHUNKS_SIZE;
    for (int sx = 0; sx < N; sx++) {
        for (int sz = 0; sz < N; sz++) {
            if (world->slot_cx[sx][sz] == INT_MIN) continue;

            /* Making a cube out of the chunks position by using the
             * furthest separated vertices. */
            chunk_t* chunk = &world->chunks[sx][sz];
            vec3 chunk_aabb[2] = {
                {chunk->position[0], chunk->position[1],
                 chunk->position[2]},
                {chunk->position[0] + CHUNK_SIZE_XZ,
                 chunk->position[1] + CHUNK_SIZE_Y,
                 chunk->position[2] + CHUNK_SIZE_XZ},
            };
            /* Checking if the cube made of the chunk intersects the
             * frustum of the camera. If yes, render the chunk. */
            if (glm_aabb_frustum(chunk_aabb, frustum))
                chunk_draw(chunk, shader, atlas);
        }
    }
}

void world_destroy(world_t* world) {
    const int N = LOADED_CHUNKS_SIZE;
    for (int sx = 0; sx < N; sx++)
        for (int sz = 0; sz < N; sz++)
            chunk_destroy(&world->chunks[sx][sz]);
}

bool world_valid_position(const world_t* world, const vec3 position) {
    if (position[1] < 0.0f || position[1] >= (float)CHUNK_SIZE_Y)
        return false;
    const int cx = (int)floorf(position[0] / (float)CHUNK_SIZE_XZ);
    const int cz = (int)floorf(position[2] / (float)CHUNK_SIZE_XZ);
    return world_get_chunk((world_t*)world, cx, cz) != NULL;
}

void world_generator_perlin(chunk_t* chunk, int world_cx, int world_cz,
                            const void* userdata) {
    const perlin_params_t* p = userdata;
    for (int lx = 0; lx < CHUNK_SIZE_XZ; lx++) {
        for (int lz = 0; lz < CHUNK_SIZE_XZ; lz++) {
            const float wx =
                (float)(world_cx * CHUNK_SIZE_XZ + lx) * p->scale;
            const float wz =
                (float)(world_cz * CHUNK_SIZE_XZ + lz) * p->scale;

            vec2 pt = {wx, wz};
            const int surface = p->sea_level + (int)(glm_perlin_vec2(pt) *
                                                     (float)p->amplitude);
            int y = 0;
            for (; y < surface - 5 && y < CHUNK_SIZE_Y; y++)
                chunk->blocks[lx][y][lz] = BLOCK_STONE;
            for (; y < surface - 4 && y < CHUNK_SIZE_Y; y++)
                chunk->blocks[lx][y][lz] = BLOCK_SAND;
            for (; y < surface - 1 && y < CHUNK_SIZE_Y; y++)
                chunk->blocks[lx][y][lz] = BLOCK_DIRT;
            if (surface > 0 && y < CHUNK_SIZE_Y)
                chunk->blocks[lx][y][lz] = BLOCK_GRASS;
        }
    }
}
