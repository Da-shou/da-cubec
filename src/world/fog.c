#include "world/fog.h"

#include "game_state.h"

void reload_fog(const game_state_t* game_state) {
    /* Setting our fog variables and sending them to the shader. The fog color will be the
     * same as the sky, fog_near is the distance from the camera from which the fog will
     * start, fog_far is when the fog will be at maximum density.*/
    const game_config_t config = game_state->config;
    const shader_t* shader     = &game_state->cube_shader;

    shader_set_vec3(
        shader, "fog_color",
        (vec3) {config.sky_color[0], config.sky_color[1], config.sky_color[2]});
    const int max_render_distance = config.render_distance * CHUNK_SIZE_XZ;
    shader_set_int(shader, "max_render_distance", max_render_distance);
    shader_set_int(shader, "render_distance", config.render_distance);
}
