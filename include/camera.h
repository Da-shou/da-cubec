#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <cglm/types.h>

#include "game_config.h"

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
 * are there for that purpose.
 * @param up Normalized up vector going from camera.
 * @param world_up Constant normalized vector indicating the world's up.
 * @param right Normalized right vector going from camera.
 * @param front Normalized front vector going from camera. Can also be seen
 * as the "direction of the camera".
 * @param position Position of the camera in global space.
 * @param yaw Rotation of the camera on the Z axis.
 * @param pitch Rotation of the camera on the Y axis.
 * @param movement_speed Defines the speed of the camera.
 * @param mouse_sensitivity Defines how much the camera rotates on mouse
 * movement.
 * @param zoom Zoom to apply to the camera if needed. */
typedef struct {
    vec3 up;
    vec3 world_up;
    vec3 right;
    vec3 front;
    vec3 position;
    float yaw;
    float pitch;
    float movement_speed;
    float mouse_sensitivity;
    float zoom;
} camera_t;

/**
 * @brief Initializes the camera vectors and puts it at the
 * orgin of the world.
 * @param config Settings to be applied to the camera
 * @param camera Camera struct to be initalized.
 * @param position Initial position of the camera in 3D space. */
void camera_init(const game_config_t* config, camera_t* camera, vec3 position);

/**
 * @brief Updates the right, front and up vectors of the camera. Has to be
 * called every time the camera is rotated, and not when the camera is
 * moved on these axis because they do not change.
 * @param camera Camera to be moved. */
void camera_update_vectors(camera_t* camera);

/**
 * @brief Updates the view matrix using the camera's attributes
 * @param camera Camera to be used in the update
 * @param view View matrix to be modified */
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
 * is very much recommanded to be set as true to forbid the camera to
 * rotate vertically.*/
void camera_rotate(camera_t* camera, float x_pos, float y_pos, GLboolean constrain_pitch);

/**
 * @brief Used to tell the camera that the mouse went out of the window so
 * that it can prevent jumping whenever the mouse comes back on the window.
 */
void camera_reset_mouse(void);
#endif
