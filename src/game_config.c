#include <game_config.h>
game_config_t game_config_default(void) {
    return (game_config_t){
        .width                = 800,
        .height               = 600,
        .title                = "da-cubec",
        .vertex_shader_path   = "src/shaders/basic.vert.glsl",
        .fragment_shader_path = "src/shaders/basic.frag.glsl",
        .texture_atlas_path   = "img/atlas.png",
        .sensitivity          = 0.05f,
        .speed                = 5.0f,
        .fov                  = 70.0f,
        .max_reach            = 6.0f,
        .sky_color            = {0.85f, 0.85f, 1.0f, 1.0f},
    };
}