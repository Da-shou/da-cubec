#include <stdbool.h>
#include <cglm/cam.h>
#include <cglm/vec3.h>

#include "camera.h"

#include "gl_glfw_include.h"

static const float camera_yaw   = -90.0F;
static const float camera_pitch = 0.0F;
static const float camera_zoom  = 45.0F;

static float last_mouse_x = 0;
static float last_mouse_y = 0;
static bool first_mouse   = true;

void camera_init(const game_config_t* config, camera_t* camera, vec3 position) {
    /* Setting up the camera's intial position.*/
    glm_vec3_copy(position, camera->position);

    /* The default front vector*/
    glm_vec3_copy((vec3) {0.0F, 0.0F, -1.0F}, camera->front);

    /* Setting the default parameter of the camera */
    camera->yaw               = camera_yaw;
    camera->pitch             = camera_pitch;
    camera->movement_speed    = config->speed;
    camera->mouse_sensitivity = config->sensitivity;
    camera->zoom              = camera_zoom;

    glm_vec3_copy((vec3) {0.0F, 1.0F, 0.0F}, camera->world_up);

    camera_update_vectors(camera);
}

void camera_update_vectors(camera_t* camera) {
    /* Calculate the front vector using calculations to take yaw and
     * pitch into account, which will allow moving the camera with the
     * mouse. */
    const float front_x = cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    const float front_y = sinf(glm_rad(camera->pitch));
    const float front_z = sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    glm_vec3_copy((vec3) {front_x, front_y, front_z}, camera->front);
    glm_normalize(camera->front);

    /* Now that the front vector has been calculated, it's very easy to
     * calculate the right and up vector from it;*/
    glm_cross(camera->world_up, camera->front, camera->right);
    glm_normalize(camera->right);
    glm_cross(camera->front, camera->right, camera->up);
    glm_normalize(camera->up);
}

void camera_update_view(camera_t* camera, mat4 view) {
    vec3 direction;
    glm_vec3_add(camera->position, camera->front, direction);
    glm_lookat(camera->position, direction, camera->up, view);
}

void camera_move(camera_t* camera, const CAMERA_DIRECTION direction,
                 const float delta_time) {
    const float camera_delta_speed = camera->movement_speed * delta_time;

    vec3 temp;
    switch (direction) {
    case CAMERA_FORWARD:
        glm_vec3_scale(camera->front, camera_delta_speed, temp);
        glm_vec3_add(camera->position, temp, camera->position);
        break;
    case CAMERA_BACKWARD:
        glm_vec3_scale(camera->front, camera_delta_speed, temp);
        glm_vec3_sub(camera->position, temp, camera->position);
        break;
    case CAMERA_UP:
        glm_vec3_scale(camera->world_up, camera_delta_speed, temp);
        glm_vec3_add(camera->position, temp, camera->position);
        break;
    case CAMERA_DOWN:
        glm_vec3_scale(camera->world_up, camera_delta_speed, temp);
        glm_vec3_sub(camera->position, temp, camera->position);
        break;
    case CAMERA_LEFT:
        glm_cross(camera->front, camera->up, temp);
        glm_normalize(temp);
        glm_vec3_scale(temp, camera_delta_speed, temp);
        glm_vec3_sub(camera->position, temp, camera->position);
        break;
    case CAMERA_RIGHT:
        glm_cross(camera->front, camera->up, temp);
        glm_normalize(temp);
        glm_vec3_scale(temp, camera_delta_speed, temp);
        glm_vec3_add(camera->position, temp, camera->position);
        break;
    }
}

void camera_rotate(camera_t* camera, const float x_pos, const float y_pos,
                   const GLboolean constrain_pitch) {
    /* Checking if the this is the first input of the mouse. If yes,
     * replace the last_mouse_positions with the new positions to avoid
     * jumping when clicking on the widow again. */
    if (first_mouse) {
        last_mouse_x = x_pos;
        last_mouse_y = y_pos;
        first_mouse  = false;
    }

    float x_offset = x_pos - last_mouse_x;
    float y_offset = y_pos - last_mouse_y;

    x_offset *= camera->mouse_sensitivity;
    y_offset *= camera->mouse_sensitivity;

    camera->yaw += x_offset;
    camera->pitch -= y_offset;

    last_mouse_x = x_pos;
    last_mouse_y = y_pos;

    /* Used to clamp the camera movement to disallow the camera to
     * completely rotate. */
    if (constrain_pitch) {
        if (camera->pitch > 89.0F) { camera->pitch = 89.0F; }
        if (camera->pitch < -89.0F) { camera->pitch = -89.0F; }
    }

    /* When the camera rotates, its front, right and up vector change
     * so they need to be recalculated.*/
    camera_update_vectors(camera);
}

void camera_reset_mouse(void) {
    first_mouse = true;
}
