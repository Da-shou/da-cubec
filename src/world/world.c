#include <cglm/cglm.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "cglm/vec3.h"
#include "world/blocks.h"
#include "world/world.h"

#include "game_config.h"

#include <time.h>

/**
 * @brief Gets the correct slot index for a chunk coordinate.
 * @param coord The chunk coordinate to translate to slot.
 * @param max_load The corrent max loaded chunk size (render distance * 2 + 1)
 * @return The slot index for this chunk coordinate.
 */
static int chunk_to_slot(const int coord, const int max_load) {
    return ((coord % max_load) + max_load) % max_load;
}

void world_init(world_t* world, const game_config_t* config) {
    srand(time(NULL));
    world->last_player_cx = INT_MIN;
    world->last_player_cz = INT_MIN;
    world->render_distance = config->render_distance;
    world->generate = NULL;
    world->generator_data = NULL;
    const int max_loaded_chunks_size = (world->render_distance * 2) + 1;
    vec3 dummy;
    glm_vec3_copy(GLM_VEC3_ZERO, dummy);
    for (int sx = 0; sx < max_loaded_chunks_size; sx++) {
        for (int sz = 0; sz < max_loaded_chunks_size; sz++) {
            chunk_init(&world->chunks[sx][sz], dummy);
            world->slot_cx[sx][sz] = INT_MIN;
            world->slot_cz[sx][sz] = INT_MIN;
        }
    }
    chunk_store_init(&world->chunk_store);
}

chunk_t* world_get_chunk(world_t* world, const int chunk_x, const int chunk_z) {
    const int max_loaded_chunk_size = (world->render_distance * 2) + 1;
    const int slot_x = chunk_to_slot(chunk_x, max_loaded_chunk_size);
    const int slot_z = chunk_to_slot(chunk_z, max_loaded_chunk_size);
    if (world->slot_cx[slot_x][slot_z] == chunk_x &&
        world->slot_cz[slot_x][slot_z] == chunk_z) {
        return &world->chunks[slot_x][slot_z];
    }
    return NULL;
}

int world_update(world_t* world, const vec3 player_pos) {
    const int render_distance = world->render_distance;
    const int max_loaded_chunk_size = (render_distance * 2) + 1;

    /* Getting chunk coordinates in which the player is right now. */
    const int pcx = (int)floorf(player_pos[0] / (float)CHUNK_SIZE_XZ);
    const int pcz = (int)floorf(player_pos[2] / (float)CHUNK_SIZE_XZ);

    /* If the player has not crossed a new chunk, do not do anything. */
    if (pcx == world->last_player_cx && pcz == world->last_player_cz) { return 0; }

    world->last_player_cx = pcx;
    world->last_player_cz = pcz;

    /* dirty[sx][sz] = true if this slot was reloaded this update.
     * Used to avoid rebuilding meshes that did not change. */
    bool dirty[MAX_LOADED_CHUNKS_SIZE][MAX_LOADED_CHUNKS_SIZE] = {0};

    for (int sx = 0; sx < max_loaded_chunk_size; sx++) {
        for (int sz = 0; sz < max_loaded_chunk_size; sz++) {
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
            const int base_cx = pcx - render_distance;
            const int base_sx = chunk_to_slot(base_cx, max_loaded_chunk_size);
            const int target_cx = base_cx + ((sx - base_sx + max_loaded_chunk_size) %
                                             max_loaded_chunk_size);

            const int base_cz = pcz - render_distance;
            const int base_sz = chunk_to_slot(base_cz, max_loaded_chunk_size);
            const int target_cz = base_cz + ((sz - base_sz + max_loaded_chunk_size) %
                                             max_loaded_chunk_size);

            /* If the target chunk for this slot is already loaded, skip.
             */
            if (world->slot_cx[sx][sz] == target_cx &&
                world->slot_cz[sx][sz] == target_cz) {
                continue;
            }

            chunk_t* slot = &world->chunks[sx][sz];

            /* Persist modified chunks before evicting. */
            if (slot->modified && world->slot_cx[sx][sz] != INT_MIN) {
                chunk_store_save(&world->chunk_store, world->slot_cx[sx][sz],
                                 world->slot_cz[sx][sz], slot->blocks);
            }

            /* Clear blocks and reset mesh counters. */
            memset(slot->blocks, 0, sizeof(slot->blocks));
            slot->mesh.vertex_count = 0;
            slot->mesh.index_count = 0;
            slot->modified = false;

            /* Update the world-space position used by chunk_draw. */
            slot->position[0] = (float)(target_cx * CHUNK_SIZE_XZ);
            slot->position[1] = 0.0F;
            slot->position[2] = (float)(target_cz * CHUNK_SIZE_XZ);

            world->slot_cx[sx][sz] = target_cx;
            world->slot_cz[sx][sz] = target_cz;

            if (!chunk_store_load(&world->chunk_store, target_cx, target_cz,
                                  slot->blocks)) {
                if (world->generate) {
                    world->generate(slot, target_cx, target_cz, world->generator_data);
                }
            }

            dirty[sx][sz] = true;
        }
    }

    /* Only rebuilding the dirty meshes that have just been built. */
    for (int sx = 0; sx < max_loaded_chunk_size; sx++) {
        for (int sz = 0; sz < max_loaded_chunk_size; sz++) {
            const int wsx = (sx - 1 + max_loaded_chunk_size) % max_loaded_chunk_size;
            const int esx = (sx + 1) % max_loaded_chunk_size;
            const int ssz = (sz - 1 + max_loaded_chunk_size) % max_loaded_chunk_size;
            const int nsz = (sz + 1) % max_loaded_chunk_size;
            if (dirty[sx][sz] || dirty[wsx][sz] || dirty[esx][sz] || dirty[sx][ssz] ||
                dirty[sx][nsz]) {
                const int memcheck = world_build_chunk(world, sx, sz);
                if (memcheck < 0) { return -1; }
            }
        }
    }

    return 0;
}

int world_build_chunk(world_t* world, const int slot_x, const int slot_z) {
    const int chunk_x = world->slot_cx[slot_x][slot_z];
    const int chunk_z = world->slot_cz[slot_x][slot_z];
    const chunk_neighbours_t neighbors = {
        .west = world_get_chunk(world, chunk_x - 1, chunk_z),
        .east = world_get_chunk(world, chunk_x + 1, chunk_z),
        .south = world_get_chunk(world, chunk_x, chunk_z - 1),
        .north = world_get_chunk(world, chunk_x, chunk_z + 1),
    };
    chunk_t* chunk = world_get_chunk(world, chunk_x, chunk_z);
    const int memcheck = chunk_build_mesh(chunk, &chunk->mesh, neighbors);
    return memcheck;
}

/**
 * @brief Checks if a chunk is loaded and rebuilds it if it is.
 * @param world World to check the chunks in
 * @param chunk_x X coordinate of the chunk in the chunk grid
 * @param chunk_z Z coordinate of the chunk in the chunk grid
 * @return 0 on success, -1 on memory allocation failure
 */
static int rebuild_if_loaded(world_t* world, const int chunk_x, const int chunk_z) {
    const int max_loaded_chunk_size = (world->render_distance * 2) + 1;
    const int slot_x = chunk_to_slot(chunk_x, max_loaded_chunk_size);
    const int slot_z = chunk_to_slot(chunk_z, max_loaded_chunk_size);
    int memcheck = 0;
    if (world->slot_cx[slot_x][slot_z] == chunk_x &&
        world->slot_cz[slot_x][slot_z] == chunk_z) {
        memcheck = world_build_chunk(world, slot_x, slot_z);
    }
    return memcheck;
}

// clang-format off
int world_rebuild_after_change(world_t* world, const int chunk_x,
                                const int chunk_z, const int local_x,
                                const int local_z) {
    int memcheck = 0;
    memcheck += rebuild_if_loaded(world, chunk_x, chunk_z);
    if (local_x == 0)
        { memcheck += rebuild_if_loaded(world, chunk_x - 1, chunk_z); }
    if (local_x == CHUNK_SIZE_XZ - 1)
        { memcheck += rebuild_if_loaded(world, chunk_x + 1, chunk_z); }
    if (local_z == 0)
        { memcheck += rebuild_if_loaded(world, chunk_x, chunk_z - 1); }
    if (local_z == CHUNK_SIZE_XZ - 1)
        { memcheck += rebuild_if_loaded(world, chunk_x, chunk_z + 1); }
    if (memcheck < 0) { return -1; }
    return 0;
}

// clang-format on

void world_draw(world_t* world, const shader_t* shader, const material_t* atlas,
                vec4 frustum[6]) {
    const int max_loaded_chunk_size = (world->render_distance * 2) + 1;
    for (int sx = 0; sx < max_loaded_chunk_size; sx++) {
        for (int sz = 0; sz < max_loaded_chunk_size; sz++) {
            if (world->slot_cx[sx][sz] == INT_MIN) { continue; }

            /* Making a cube out of the chunks position by using the
             * furthest separated vertices. */
            chunk_t* chunk = &world->chunks[sx][sz];
            vec3 chunk_aabb[2] = {
                {chunk->position[0], chunk->position[1], chunk->position[2]},
                {chunk->position[0] + CHUNK_SIZE_XZ, chunk->position[1] + CHUNK_SIZE_Y,
                 chunk->position[2] + CHUNK_SIZE_XZ},
            };

            /* Checking if the cube made of the chunk intersects the
             * frustum of the camera. If yes, render the chunk. */
            if (glm_aabb_frustum(chunk_aabb, frustum)) {
                chunk_draw(chunk, shader, atlas);
            }
        }
    }
}

void world_destroy(world_t* world) {
    const int max_loaded_chunk_size = MAX_LOADED_CHUNKS_SIZE;
    for (int sx = 0; sx < max_loaded_chunk_size; sx++) {
        for (int sz = 0; sz < max_loaded_chunk_size; sz++) {
            chunk_destroy(&world->chunks[sx][sz]);
        }
    }
    chunk_store_destroy(&world->chunk_store);
}

uint8_t world_get_block(world_t* world, const int block_x, const int block_y,
                        const int block_z) {
    if (block_y < 0 || block_y >= CHUNK_SIZE_Y) { return BLOCK_AIR; }
    const int chunk_x = (int)floorf((float)block_x / (float)CHUNK_SIZE_XZ);
    const int chunk_z = (int)floorf((float)block_z / (float)CHUNK_SIZE_XZ);
    const chunk_t* chunk = world_get_chunk(world, chunk_x, chunk_z);
    if (chunk == NULL) { return BLOCK_AIR; }
    const int local_x = block_x - (chunk_x * CHUNK_SIZE_XZ);
    const int local_z = block_z - (chunk_z * CHUNK_SIZE_XZ);
    return chunk->blocks[local_x][block_y][local_z];
}

bool world_valid_position(const world_t* world, const vec3 position) {
    if (position[1] < 0.0F || position[1] >= (float)CHUNK_SIZE_Y) { return false; }
    const int chunk_x = (int)floorf(position[0] / (float)CHUNK_SIZE_XZ);
    const int chunk_z = (int)floorf(position[2] / (float)CHUNK_SIZE_XZ);
    return world_get_chunk((world_t*)world, chunk_x, chunk_z) != NULL;
}

void world_generator_perlin(chunk_t* chunk, const int world_cx, const int world_cz,
                            const void* userdata) {
    const perlin_params_t* perlin_params = userdata;
    for (int lx = 0; lx < CHUNK_SIZE_XZ; lx++) {
        for (int lz = 0; lz < CHUNK_SIZE_XZ; lz++) {
            const float world_x =
                (float)((world_cx * CHUNK_SIZE_XZ) + lx) * perlin_params->scale;
            const float world_z =
                (float)((world_cz * CHUNK_SIZE_XZ) + lz) * perlin_params->scale;

            /* surface is the highest y point of the column of blocks
             * currently being constructed. */
            vec2 perlin_point = {world_x, world_z};
            const int surface =
                perlin_params->sea_level +
                (int)(glm_perlin_vec2(perlin_point) * (float)perlin_params->amplitude);

            /* Setting the different layers of block depending on the
             * y level */
            int altitude = 0;
            for (; altitude < surface - 5 && altitude < CHUNK_SIZE_Y; altitude++) {
                chunk->blocks[lx][altitude][lz] = BLOCK_STONE;
            }
            for (; altitude < surface - 4 && altitude < CHUNK_SIZE_Y; altitude++) {
                chunk->blocks[lx][altitude][lz] = BLOCK_SAND;
            }
            for (; altitude < surface - 1 && altitude < CHUNK_SIZE_Y; altitude++) {
                chunk->blocks[lx][altitude][lz] = BLOCK_DIRT;
            }
            if (surface > 0 && altitude < CHUNK_SIZE_Y) {
                chunk->blocks[lx][altitude][lz] = BLOCK_GRASS;
            }
        }
    }
}
