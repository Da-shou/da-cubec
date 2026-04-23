/**
 * @file camera.h
 * @authors Da-shou
 * Header file containing all of the declaration of the functions and enums controlling
 * a camera. Also describes the camera_t structure and all of its properties.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/types.h>
#include "game_config.h"
#include "opengl_glfw.h"

/**
 * @brief Helper enum to make the moving of the camera more readable. */
typedef enum {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
} CAMERA_DIRECTION;

/**
 * @brief Camera structure storing all of its important vectors. The values
 * of the camera are usually not manually modified. The helper functions
 * are there for that purpose. */
typedef struct {
    vec3 up;              /**< Direction vector pointing up from the camera. */
    vec3 world_up;        /**< Direction vector pointing up from the world's origin. */
    vec3 right;           /**< Direction vector pointing right from the camera. */
    vec3 front;           /**< Direction vector pointing at the camera target. */
    vec3 position;        /**< Coordinates of the camera in world coordinates. */
    float yaw;            /**< Yaw value of the rotation of the camera. */
    float pitch;          /**< Pitch value of the rotation of the camera. */
    float movement_speed; /**< Movement speed used when the camera is in freecam mode. */
    float mouse_sensitivity; /**< Sensitivity of the camera rotation using the mouse
    controls */
    float zoom;              /** How much the camera is zoomed in. */
} camera_t;

/**
 * @brief Initializes the camera vectors and puts it at the
 * orgin of the world.
 * @param config Settings to be applied to the camera
 * @param camera Camera struct to be initalized.
 * @param position Initial position of the camera in 3D space.
 */
void camera_init(const game_config_t* config, camera_t* camera, vec3 position);

/**
 * @brief Updates the right, front and up vectors of the camera. Has to be
 * called every time the camera is rotated, and not when the camera is
 * moved on these axis because they do not change.
 * @param camera Camera to be moved.
 */
void camera_update_vectors(camera_t* camera);

/**
 * @brief Updates the view matrix using the camera's attributes
 * @param camera Camera to be used in the update
 * @param view View matrix to be modified
 */
void camera_update_view(camera_t* camera, mat4 view);

/**
 * @brief Moves the camera forward, backwards, to the right or left.
 * @param camera Camera to be moved.
 * @param direction Member of CAMERA_DIRECTION enum to specify direction.
 * @param delta_time Time delta to multiply velocity with to be constant
 * between frames.
 */
void camera_move(camera_t* camera, CAMERA_DIRECTION direction, float delta_time);

/**
 * @brief Makes the camera rotate following its yaw and pitch attributes
 * based on the x and y offset provided by GLFW mouse support.
 * @param camera Camera to be rotated
 * @param x_pos X Offset to apply to the yaw
 * @param y_pos Y Offset to apply to the pitch
 * @param constrain_pitch If the pitch needs to be contrained or not. This
 * @attention is very much recommanded to be set as true to forbid the camera to
 * rotate vertically.
 */
void camera_rotate(camera_t* camera, float x_pos, float y_pos, GLboolean constrain_pitch);

/**
 * @brief Used to tell the camera that the mouse went out of the window so
 * that it can prevent jumping whenever the mouse comes back on the window.
 */
void camera_reset_mouse(void);
#endif
