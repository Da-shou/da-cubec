/**
 * @file game_config.h
 * @authors Da-shou
 * Header file describing a structure containing most attributes that can be modified
 * to alter the game's behaviour, as well as the file paths of all the external files
 * the games needs to load.
 */

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

    /** Scale of the down vector to add each frame to a
     * player not touching the ground */
    float gravity;

    /** Width of the rectangular hitbox around the player. 1.0F = 1 block. */
    float player_width;

    /** Height of the rectangular hitbox around the player. 1.0F = 1 block.*/
    float player_height;

    float player_eye_offset; /**< How high the eyes are, starting from the ground */
    float terminal_velocity; /**< Maximum falling speed */

    /* World settings */
    /** Defines how many blocks of range the player has to break or place blocks. */
    float max_reach;
    /** Defines the sky color. The fog copies these values too. */
    float sky_color[4];

    /* HUD */
    /** Font size of the text used to display debug info on
    screen */
    float debug_font_size;
} game_config_t;

/**
 * @brief Returns the default configuration settings for the game.
 * @return Config - Default configuration settings
 */
game_config_t game_config_default(void);

#endif // GAME_CONFIG_H
