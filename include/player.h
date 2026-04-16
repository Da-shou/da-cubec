#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <cglm/types.h>

#include "camera.h"
#include "game_config.h"
#include "world/world.h"

#define COLLISION_EPSILON 0.0001F

/**
 * Defines all attributes of a player.
 */
typedef struct {
    vec3 position;      /* Feet position (bottom-center of AABB) */
    vec3 velocity;      /* Current velocity (x, y, z) */
    float width;        /* AABB width and depth (centered on position XZ) */
    float height;       /* AABB height (extends upward from position.y) */
    float eye_offset;   /* Eye height above feet */
    bool on_ground;     /* True when standing on a solid surface */
} player_t;

/**
 * Initializes a player with the given configuration and spawn position.
 * @param player Pointer to player struct to update.
 * @param config Pointer to the current configuration of the game to get the player's attributes.
 * @param spawn Spawn position of the player.
 */
void player_init(player_t* player, const game_config_t* config, vec3 spawn);

/**
 * Updates the player each frame, processing the wished movements of the player.
 * @param player Pointer to player struct to update
 * @param config Pointer to the current configuration of the game to get the player's attributes.
 * @param world Pointer to the world the player is in to compute collisions.
 * @param camera Pointer to the camera the player is using to view the world.
 * @param wish_forward How much the player wants to move forward.
 * @param wish_right How much hthe player wants to move right.
 * @param jump_pressed True if the players wants a jump.
 * @param sprint True if the player should be sprinting.
 * @param delta_time
 */
void player_update(player_t* player, const game_config_t* config,
                   world_t* world, camera_t* camera,
                   float wish_forward, float wish_right,
                   bool jump_pressed, bool sprint, float delta_time);

#endif /* PLAYER_H */