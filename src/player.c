#include <math.h>
#include <cglm/cglm.h>

#include "player.h"
#include "world/world.h"
#include "world/blocks.h"
#include "camera.h"

void player_init(player_t* player, const game_config_t* config, camera_t* camera,
                 vec3 spawn) {
    glm_vec3_copy(spawn, player->position);
    glm_vec3_zero(player->velocity);
    player->width      = config->player_width;
    player->height     = config->player_height;
    player->eye_offset = config->player_eye_offset;
    player->camera     = camera;
    player->on_ground  = false;
}

/**
 * @brief Check if the player AABB at a given position overlaps any solid block.
 * The AABB is defined by feet_pos (bottom center), width, and height.
 * @return true if collision detected, false otherwise
 */
static bool player_collides(world_t* world, const float player_x, const float player_y,
                            const float player_z, const float half_w,
                            const float height) {
    /* Making a box around the player. So we can interate on each block
     * afterwards and if at least one block is not BLOCK_AIR, then there is a collision.
     */
    const int min_bx = (int)floorf(player_x - half_w);
    const int max_bx = (int)floorf(player_x + half_w - COLLISION_EPSILON);
    const int min_by = (int)floorf(player_y);
    const int max_by = (int)floorf(player_y + height - COLLISION_EPSILON);
    const int min_bz = (int)floorf(player_z - half_w);
    const int max_bz = (int)floorf(player_z + half_w - COLLISION_EPSILON);

    for (int bx = min_bx; bx <= max_bx; bx++) {
        for (int by = min_by; by <= max_by; by++) {
            for (int bz = min_bz; bz <= max_bz; bz++) {
                if (world_get_block(world, bx, by, bz) != BLOCK_AIR) { return true; }
            }
        }
    }

    return false;
}

void player_update(player_t* player, const game_config_t* config, world_t* world,
                   camera_t* camera, const float wish_forward, const float wish_right,
                   const bool jump_pressed, const bool sprint, const float delta_time) {
    /* Horizontal velocity from input. We need to make flat vectors from the front vector
     * of the camera, putting the y axis at 0.0F.*/
    vec3 flat_front       = {camera->front[0], 0.0F, camera->front[2]};
    const float front_len = glm_vec3_norm(flat_front);
    if (front_len > COLLISION_EPSILON) {
        glm_vec3_scale(flat_front, 1.0F / front_len, flat_front);
    }

    /* Same for the right vector */
    vec3 flat_right       = {camera->right[0], 0.0F, camera->right[2]};
    const float right_len = glm_vec3_norm(flat_right);
    if (right_len > COLLISION_EPSILON) {
        glm_vec3_scale(flat_right, 1.0F / right_len, flat_right);
    }

    /* Wished direction of the player, multiplied by the flat vector values from before.
     */
    vec3 wished_direction = {
        (wish_forward * flat_front[0]) + (wish_right * flat_right[0]), 0.0F,
        (wish_forward * flat_front[2]) + (wish_right * flat_right[2])};

    /* Length of the wished direction */
    const float wish_len = glm_vec3_norm(wished_direction);

    /* Setting correct player speed */
    const float speed = sprint ? config->player_sprint_speed : config->player_walk_speed;

    if (wish_len > COLLISION_EPSILON) {
        glm_vec3_scale(wished_direction, speed / wish_len, wished_direction);
        player->velocity[0] = wished_direction[0];
        player->velocity[2] = wished_direction[2];
    } else {
        player->velocity[0] = 0.0F;
        player->velocity[2] = 0.0F;
    }

    /* Jump */
    if (jump_pressed && player->on_ground) {
        player->velocity[1] = config->player_jump_velocity;
        player->on_ground   = false;
    }

    /* Creating the gravity vector, and clamping the value on the player's terminal
     * velocity. */
    if (!player->on_ground) {
        player->velocity[1] -= config->gravity * delta_time;
        if (player->velocity[1] < -config->terminal_velocity) {
            player->velocity[1] = -config->terminal_velocity;
        }
    }

    /* Per-axis collision resolution */
    const float half_w = player->width / 2.0F;
    float player_x     = player->position[0];
    float player_y     = player->position[1];
    float player_z     = player->position[2];

    /* Y axis (resolve first to establish on_ground) */
    const float delta_y_velocity = player->velocity[1] * delta_time;

    player->on_ground = false;
    if (delta_y_velocity != 0.0F) {
        float new_py = player_y + delta_y_velocity;
        if (player_collides(world, player_x, new_py, player_z, half_w, player->height)) {
            if (delta_y_velocity < 0.0F) {
                new_py            = floorf(new_py) + 1.0F;
                player->on_ground = true;
            } else {
                new_py = floorf(new_py + player->height) - player->height;
            }
            player->velocity[1] = 0.0F;
        }
        player_y = new_py;
    }

    /* X axis */
    const float delta_x_velocity = player->velocity[0] * delta_time;
    if (delta_x_velocity != 0.0F) {
        float new_px = player_x + delta_x_velocity;
        if (player_collides(world, new_px, player_y, player_z, half_w, player->height)) {
            if (delta_x_velocity > 0.0F) {
                new_px = floorf(new_px + half_w) - half_w;
            } else {
                new_px = floorf(new_px - half_w) + 1.0F + half_w;
            }
            player->velocity[0] = 0.0F;
        }
        player_x = new_px;
    }

    /* Z axis */
    const float delta_z_velocity = player->velocity[2] * delta_time;
    if (delta_z_velocity != 0.0F) {
        float new_pz = player_z + delta_z_velocity;
        if (player_collides(world, player_x, player_y, new_pz, half_w, player->height)) {
            if (delta_z_velocity > 0.0F) {
                new_pz = floorf(new_pz + half_w) - half_w;
            } else {
                new_pz = floorf(new_pz - half_w) + 1.0F + half_w;
            }
            player->velocity[2] = 0.0F;
        }
        player_z = new_pz;
    }

    /* Commit position  */
    glm_vec3_copy((vec3) {player_x, player_y, player_z}, player->position);

    /* Check if player has fallen under the map */
    if (player->position[1] < 0.0F) {
        for (int16_t by = CHUNK_SIZE_Y; by > 0; --by) {
            switch (world_get_block(world, 0, by, 0)) {
            case BLOCK_AIR: continue;
            default:
                glm_vec3_copy(
                    (vec3) {
                        0.0F,
                        (float)by + 1.0F,
                        0.0F,
                    },
                    player->position);
                break;
            }
            if (player->position[1] > 0.0F) { break; }
        }
    }

    /* Update camera to be at player position  */
    glm_vec3_copy(player->position, camera->position);
    camera->position[1] += player->eye_offset;
}