// app_context.h
#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <stdbool.h>

#include "camera.h"
#include "game_config.h"
#include "player.h"

/** This struct regroups the most accessed elements during runtime
 * for ease of access by the other functions. */
typedef struct {
    bool        focused;
    camera_t*   main_camera;
    shader_t* cube_shader;
    game_config_t* config;
    vec3* target_block;
    vec3* neighbour_block;
    player_t*   player;
} game_context_t;

void set_game_context(game_context_t* game_context);
game_context_t* get_game_context(void);

#endif // GAME_CONTEXT_H