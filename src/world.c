#include <cglm/cglm.h>
#include <math.h>

#include "world.h"

void world_init(world_t* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int z = 0; z < WORLD_SIZE_Z; z++) {
            vec3 position = {(float)x * CHUNK_SIZE_XZ, 0.0f,
                             (float)z * CHUNK_SIZE_XZ};
            chunk_init(&world->chunks[x][z], position);
        }
    }
}

void world_build(world_t* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++)
        for (int z = 0; z < WORLD_SIZE_Z; z++) {
            world_build_chunk(world, x, z);
        }
}

void world_build_chunk(world_t* world, const int cx, const int cz) {
    const chunk_neighbours_t neighbors = {
        .west = (cx > 0) ? &world->chunks[cx - 1][cz] : NULL,
        .east =
            (cx < WORLD_SIZE_X - 1) ? &world->chunks[cx + 1][cz] : NULL,
        .south = (cz > 0) ? &world->chunks[cx][cz - 1] : NULL,
        .north =
            (cz < WORLD_SIZE_Z - 1) ? &world->chunks[cx][cz + 1] : NULL,
    };
    chunk_t* chunk = &world->chunks[cx][cz];
    chunk_build_mesh(chunk, &chunk->mesh, neighbors);
}

void world_rebuild_after_change(world_t* world, const int chunk_x,
                                const int chunk_z, const int local_x,
                                const int local_z) {
    /* Building the centre chunk, where the camera is */
    world_build_chunk(world, chunk_x, chunk_z);

    /* Rebuilding the correct chunk depending on which edge the camera
     * is located at (If it is at an edge) */
    if (local_x == 0 && chunk_x > 0)
        world_build_chunk(world, chunk_x - 1, chunk_z);
    if (local_x == CHUNK_SIZE_XZ - 1 && chunk_x < WORLD_SIZE_X - 1)
        world_build_chunk(world, chunk_x + 1, chunk_z);
    if (local_z == 0 && chunk_z > 0)
        world_build_chunk(world, chunk_x, chunk_z - 1);
    if (local_z == CHUNK_SIZE_XZ - 1 && chunk_z < WORLD_SIZE_Z - 1)
        world_build_chunk(world, chunk_x, chunk_z + 1);
}

void world_draw(world_t* world, const shader_t* shader, material_t* atlas,
                vec4 frustum[6]) {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int z = 0; z < WORLD_SIZE_Z; z++) {
            chunk_t* chunk = &world->chunks[x][z];

            /* Making a cube out of the chunk's
             * position and size. */
            vec3 chunk_aabb[2] = {
                {chunk->position[0],                 chunk->position[1],
                 chunk->position[2]                },
                {chunk->position[0] + CHUNK_SIZE_XZ,
                 chunk->position[1] + CHUNK_SIZE_Y,
                 chunk->position[2] + CHUNK_SIZE_XZ},
            };

            /* Checks if the cube is in the frustum of the
             * camera. If yes, draw the chunk. If not,
             * skip the chunk rendering.
             */
            if (glm_aabb_frustum(chunk_aabb, frustum))
                chunk_draw(chunk, shader, atlas);
        }
    }
}

void world_destroy(world_t* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++)
        for (int z = 0; z < WORLD_SIZE_Z; z++)
            chunk_destroy(&world->chunks[x][z]);
}

bool world_valid_position(const vec3 position) {
    const int chunk_x = (int)floorf((position[0] / CHUNK_SIZE_XZ));
    const int chunk_z = (int)floorf((position[2] / CHUNK_SIZE_XZ));

    if (chunk_x < 0 || chunk_x >= WORLD_SIZE_X) return false;
    if (chunk_z < 0 || chunk_z >= WORLD_SIZE_Z) return false;
    if (position[1] < 0 || position[1] >= CHUNK_SIZE_Y) return false;

    return true;
}

// clang-format off
void world_fill_perlin(world_t* world, const float scale,
    const int sea_level, const int amplitude) {
    for (int x = 0; x < WORLD_SIZE_X; x++)
    for (int z = 0; z < WORLD_SIZE_Z; z++) {
        chunk_t* chunk = &world->chunks[x][z];
        for (int cx = 0; cx < CHUNK_SIZE_XZ; cx++)
        for (int cz = 0; cz < CHUNK_SIZE_XZ; cz++) {
            /* First the chunk-local coordinates
             * are scaled to world-coordinates. */
            const float wx =
                (float)(x * CHUNK_SIZE_XZ + cx) * scale;
            const float wz =
                (float)(z * CHUNK_SIZE_XZ + cz) * scale;

            /* We then use the x and z coordinates
             * to generate noise for the chunk column. */
            vec2 p = {wx, wz};
            const float noise = glm_perlin_vec2(p);

            /* The noise value is scaled to the chunk column height. */
            const int surface =
                sea_level + (int)(noise * (float)amplitude);

            /* The column is constructed layer by layer. */
            int y = 0;
            for (; y < surface - 5 && y < CHUNK_SIZE_Y; y++)
                chunk->blocks[cx][y][cz] = BLOCK_STONE;
            for (; y < surface - 4 && y < CHUNK_SIZE_Y; y++)
                chunk->blocks[cx][y][cz] = BLOCK_SAND;
            for (; y < surface - 1 && y < CHUNK_SIZE_Y; y++)
                chunk->blocks[cx][y][cz] = BLOCK_DIRT;
            if (surface > 0 && y < CHUNK_SIZE_Y)
                chunk->blocks[cx][y][cz] = BLOCK_GRASS;
        }
    }
}

void world_fill_superflat(world_t* world) {
    for (int x = 0; x < WORLD_SIZE_X; x++)
    for (int z = 0; z < WORLD_SIZE_Z; z++) {
        chunk_t* chunk = &world->chunks[x][z];
        for (int cx = 0; cx < CHUNK_SIZE_XZ; cx++)
        for (int cz = 0; cz < CHUNK_SIZE_XZ; cz++) {
            int height = 0;

            // Stone layers
            for (; height < 10; height++) {
                chunk->blocks[cx][height][cz] = BLOCK_STONE;
            }

            // Sand layer
            chunk->blocks[cx][height++][cz] = BLOCK_SAND;

            // Dirt layers
            for (; height < 13; height++) {
                chunk->blocks[cx][height][cz] = BLOCK_DIRT;
            }

            // Grass layer
            chunk->blocks[cx][height][cz] = BLOCK_GRASS;
        }
    }
}

// clang-format on
