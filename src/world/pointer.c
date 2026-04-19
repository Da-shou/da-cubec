#include <math.h>
#include <cglm/cglm.h>

#include "world/pointer.h"
#include "game_state.h"
#include "world/world.h"
#include "world/blocks.h"

uint8_t get_pointed_block(game_state_t* game_state, float max_distance) {
    vec3 pos;
    vec3 dir;
    glm_vec3_copy(game_state->main_camera->position, pos);
    glm_vec3_copy(game_state->main_camera->front, dir);

    axis_t last_pointed_to = X;

    /* Storing the camera position */
    int camera_x = (int)floorf(pos[0]);
    int camera_y = (int)floorf(pos[1]);
    int camera_z = (int)floorf(pos[2]);

    /* Seeing which directions needs to step positively or negatively
     * based on the direction vector. */
    const int step_x = (dir[0] > 0) ? 1 : -1;
    const int step_y = (dir[1] > 0) ? 1 : -1;
    const int step_z = (dir[2] > 0) ? 1 : -1;

    /* This tells us how far we need to travel along the raycast until
     * we reach the next x, y or z plane. The delta variables tells us
     * exactly how much we need to move along the ray path to cover ONE
     * unit of distance.*/
    const float delta_x = fabsf(1.0F / dir[0]);
    const float delta_y = fabsf(1.0F / dir[1]);
    const float delta_z = fabsf(1.0F / dir[2]);

    /* Then, we calculate the total distance the ray has to travel
     * starting from the camera to hit the very next vertical or
     * horizontal grid line.*/
    float side_x = (step_x > 0) ? ((float)camera_x + 1.0F - pos[0]) * delta_x
                                : (pos[0] - (float)camera_x) * delta_x;
    float side_y = (step_y > 0) ? ((float)camera_y + 1.0F - pos[1]) * delta_y
                                : (pos[1] - (float)camera_y) * delta_y;
    float side_z = (step_z > 0) ? ((float)camera_z + 1.0F - pos[2]) * delta_z
                                : (pos[2] - (float)camera_z) * delta_z;

    float distance = 0.0F;
    while (distance < max_distance) {
        /* We find the chunk in which the xyz coordinates are in
         * right now. */
        const int chunk_x = (int)floorf((float)camera_x / CHUNK_SIZE_XZ);
        const int chunk_z = (int)floorf((float)camera_z / CHUNK_SIZE_XZ);
        const int local_x = camera_x - (chunk_x * CHUNK_SIZE_XZ);
        const int local_z = camera_z - (chunk_z * CHUNK_SIZE_XZ);

        if (camera_y >= 0 && camera_y < CHUNK_SIZE_Y) {
            chunk_t* chunk = world_get_chunk(game_state->world, chunk_x, chunk_z);
            if (chunk == NULL) { continue; }
            const uint8_t block = chunk->blocks[local_x][camera_y][local_z];
            if (block != (uint8_t)BLOCK_AIR) {
                process_block((vec3) {(float)camera_x, (float)camera_y, (float)camera_z},
                              last_pointed_to, &game_state->target_block,
                              &game_state->neighbour_block, step_x, step_y, step_z);

                /* Setting the chunk pointer for the targeted block chunk
                 * and for it's neighbour, in case the neighbour is in
                 * a different chunk.*/
                game_state->target_chunk = chunk;
                const int nb_chunk_x =
                    (int)floorf((game_state->neighbour_block)[0] / CHUNK_SIZE_XZ);
                const int nb_chunk_z =
                    (int)floorf((game_state->neighbour_block)[2] / CHUNK_SIZE_XZ);

                game_state->neighbour_chunk =
                    world_get_chunk(game_state->world, nb_chunk_x, nb_chunk_z);
                return block;
            }
        }

        /* Stepping in the direction with the smallest distance to
         * a plane then updating the last plane touched */
        if (side_x < side_y) {
            if (side_x < side_z) {
                side_x += delta_x;
                camera_x += step_x;
                distance = side_x;
                last_pointed_to = X;
            } else {
                side_z += delta_z;
                camera_z += step_z;
                distance = side_z;
                last_pointed_to = Z;
            }
        } else {
            if (side_y < side_z) {
                side_y += delta_y;
                camera_y += step_y;
                distance = side_y;
                last_pointed_to = Y;
            } else {
                side_z += delta_z;
                camera_z += step_z;
                distance = side_z;
                last_pointed_to = Z;
            }
        }
    }

    return BLOCK_AIR;
}

void process_block(vec3 block_position, const axis_t last, vec3* pointed_block,
                   vec3* neighbour_block, const int step_x, const int step_y,
                   const int step_z) {
    glm_vec3_copy(block_position, *pointed_block);
    glm_vec3_copy(*pointed_block, *neighbour_block);
    switch (last) {
    case X: (*neighbour_block)[0] -= (float)step_x; break;
    case Y: (*neighbour_block)[1] -= (float)step_y; break;
    case Z: (*neighbour_block)[2] -= (float)step_z; break;
    }
}
