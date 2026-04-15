#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"
#include "world.h"
#include "game_config.h"

float delta_time = 0.0f;
float last_frame = 0.0f;

/**
 * @brief Managing inputs for mouse and keyboard. */
void handle_camera_mouse(GLFWwindow* window, const game_config_t* config,
                         camera_t* camera) {
    const float current_frame = (float)glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_move(camera, CAMERA_FORWARD, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_move(camera, CAMERA_BACKWARD, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_move(camera, CAMERA_LEFT, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_move(camera, CAMERA_RIGHT, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera_move(camera, CAMERA_UP, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera_move(camera, CAMERA_DOWN, delta_time);
    }

    const int ctrl_state = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    if (ctrl_state == GLFW_PRESS)
        camera->movement_speed = config->speed * 5;
    else camera->movement_speed = config->speed;
}

void handle_clicks(GLFWwindow* window, world_t* world,
                   const vec3 target_block, const vec3 neighbour,
                   chunk_t* target_chunk, chunk_t* neighbour_chunk) {
    static int last_lc_state = GLFW_RELEASE;
    static int last_rc_state = GLFW_RELEASE;

    const int lc_state =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    const int rc_state =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    /* Left-click -> The block gets destroyed (replaced with air) */
    if (lc_state == GLFW_PRESS && last_lc_state == GLFW_RELEASE) {
        const int lx = (int)target_block[0] % CHUNK_SIZE_XZ;
        const int ly = (int)target_block[1];
        const int lz = (int)target_block[2] % CHUNK_SIZE_XZ;
        const int cx = (int)target_block[0] / CHUNK_SIZE_XZ;
        const int cz = (int)target_block[2] / CHUNK_SIZE_XZ;

        /** Destroying the block that is being looked at **/
        target_chunk->blocks[lx][ly][lz] = (uint8_t)BLOCK_AIR;
        world_rebuild_after_change(world, cx, cz, lx, lz);
    }

    /* Right-click -> A block is placed at the neighbour coordinates.
     */
    if (rc_state == GLFW_PRESS && last_rc_state == GLFW_RELEASE) {
        if (!world_valid_position(neighbour)) return;
        const int lx = (int)neighbour[0] % CHUNK_SIZE_XZ;
        const int ly = (int)neighbour[1];
        const int lz = (int)neighbour[2] % CHUNK_SIZE_XZ;
        const int cx = (int)neighbour[0] / CHUNK_SIZE_XZ;
        const int cz = (int)neighbour[2] / CHUNK_SIZE_XZ;

        if (neighbour_chunk->blocks[lx][ly][lz] != (uint8_t)BLOCK_AIR) return;
        neighbour_chunk->blocks[lx][ly][lz] = (uint8_t)BLOCK_COBBLESTONE;
        world_rebuild_after_change(world, cx, cz, lx, lz);
    }

    last_lc_state = lc_state;
    last_rc_state = rc_state;
}
