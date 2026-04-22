#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configuration constants and runtime settings for the application.
 */
typedef struct {
    /* Window Settings */
    uint16_t width;      /**< Width of the game window. */
    uint16_t height;     /**< Height of the game window. */
    const char* title;   /**< Title of the game window. */
    const char* version; /**< Version of the game. */

    /* External files settings */
    const char* cube_vertex_shader_path;   /**< Cube vertex shader filepath. */
    const char* cube_fragment_shader_path; /**< Cube fragment shader filepath. */
    const char* text_vertex_shader_path;   /**< Text vertex shader filepath. */
    const char* text_fragment_shader_path; /**< Text fragment shader filepath. */
    const char* debug_font_path;           /**< Debug font filepath. */
    const char* texture_atlas_path;        /**< Texture image atlas. */

    /* Camera settings */
    float sensitivity;       /**< Sensitivity of the camera. */
    float speed;             /**< Movement speed of the camera. */
    float fov;               /**< FOV of the camera. */
    bool free_camera;        /**< If true, game will be in freecam mode. */
    uint8_t render_distance; /**< Render distance in chunk radius. */

    /* Player physics settings */
    float player_walk_speed;    /**< Speed of the player when walking. */
    float player_sprint_speed;  /**< Speed of the player when sprinting. */
    float player_jump_velocity; /**< Vertical speed added when jumping. */

    float gravity;              /**< Vertical vector to add each frame to a player not
    touching the ground. */

    float player_width;
    float player_height;
    float player_eye_offset;
    float terminal_velocity;

    /* World settings */
    float max_reach;
    float sky_color[4];

    /* HUD */
    float debug_font_size;
} game_config_t;

/**
 * @brief Returns the default configuration settings for the game.
 * @return Config - Default configuration settings
 */
game_config_t game_config_default(void);

#endif // GAME_CONFIG_H
