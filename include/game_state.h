/**
 * @file game_state.h
 * @authors Da-shou
 * Header file describing a structure that will only be initailized once
 * statically and contains pointers and structs to the main elements of the game, such
 * as the world, the camera, the player and others.
 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "material.h"
#include "player.h"
#include "text_renderer.h"
#include "world/world.h"

/**
 * @brief Structure regrouping pointers and structures of the main game elements. Can
 * be passed to a function to pass many useful parameters.
 */
typedef struct {
    game_config_t config; /**< Current game configuration structure */
    shader_t cube_shader; /**< Current shader program used for cube rendering */
    material_t atlas; /**< OpenGL material structure containing the atlas as a texture */
    text_renderer_t debug_text_renderer; /**< Text renderer to use for debug outputs */
    world_t* world;                      /**< Pointer to the world the game has to load */
    player_t* player;                    /** Pointer to the player the game simulates */
    vec3 target_block;
    chunk_t* target_chunk;
    vec3 neighbour_block;
    chunk_t* neighbour_chunk;
    int view_location;
    int projection_location;
    bool focused;
} game_state_t;

#endif