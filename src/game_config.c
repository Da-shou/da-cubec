#include "game_config.h"

game_config_t game_config_default(void) {
    return (game_config_t) {.width = 800,
                            .height = 600,
                            .title = "da-cubec",
                            .vertex_shader_path = "src/shaders/basic.vert.glsl",
                            .fragment_shader_path = "src/shaders/basic.frag.glsl",
                            .texture_atlas_path = "img/atlas.png",
                            .sensitivity = 0.05F,
                            .speed = 25.0F,
                            .fov = 70.0F,
                            .max_reach = 6.0F,
                            .sky_color = {0.85F, 0.85F, 1.0F, 1.0F},
                            .render_distance = 8};
}
