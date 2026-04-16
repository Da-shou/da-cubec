#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/**
 * @brief Configuration constants and runtime settings for the application.
 */
typedef struct {
    /* Window settings */
    uint16_t width, height;
    const char* title;
    const char* version;

    /* Filepath for assets */
    const char* basic_vertex_shader_path;
    const char* basic_fragment_shader_path;
    const char* text_vertex_shader_path;
    const char* text_fragment_shader_path;
    const char* font_path;
    const char* texture_atlas_path;

    /* Camera settings */
    float sensitivity;
    float speed;
    float fov;
    uint8_t render_distance;

    /* Player physics settings */
    float player_walk_speed;
    float player_sprint_speed;
    float player_jump_velocity;
    float gravity;
    float player_width;
    float player_height;
    float player_eye_offset;
    float terminal_velocity;

    /* World settings */
    float max_reach;
    float sky_color[4];
} game_config_t;

/**
 * @brief Returns the default configuration settings for the game.
 * @return Config - Default configuration settings
 */
game_config_t game_config_default(void);

#endif // CONFIG_H
