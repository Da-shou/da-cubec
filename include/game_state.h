#ifndef GAME_STATE_H
#define GAME_STATE_H
#include "material.h"
#include "player.h"
#include "text_renderer.h"
#include "world/world.h"

typedef struct {
    game_config_t config;
    shader_t cube_shader;
    material_t atlas;
    text_renderer_t debug_text_renderer;
    world_t* world;
    player_t* player;
    camera_t* main_camera;
    vec3 target_block;
    chunk_t* target_chunk;
    vec3 neighbour_block;
    chunk_t* neighbour_chunk;
    int view_location;
    int projection_location;
    bool focused;
} game_state_t;

#endif