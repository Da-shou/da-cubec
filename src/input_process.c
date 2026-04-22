#include "gl_glfw_include.h"
#include "camera.h"
#include "world/blocks.h"
#include "world/chunk.h"
#include "world/world.h"
#include "game_config.h"
#include "player.h"
#include "cglm/box.h"

#include <math.h>

void handle_debug_inputs(GLFWwindow* window, game_config_t* config, world_t* world) {
    static int last_tab_state = GLFW_RELEASE;
    const int tab_state       = glfwGetKey(window, GLFW_KEY_TAB);
    if (last_tab_state != tab_state && tab_state == GLFW_PRESS &&
        last_tab_state == GLFW_RELEASE) {
        config->free_camera = !config->free_camera;
    }
    last_tab_state = tab_state;

    static int last_pgup_state = GLFW_RELEASE;
    const int pgup_state       = glfwGetKey(window, GLFW_KEY_PAGE_UP);
    if (last_pgup_state != pgup_state && pgup_state == GLFW_PRESS &&
        last_pgup_state == GLFW_RELEASE) {
        if (config->render_distance < MAX_RENDER_DISTANCE) {
            ++(config->render_distance);
        }
    }
    last_pgup_state = pgup_state;

    static int last_pgdown_state = GLFW_RELEASE;
    const int pgdown_state       = glfwGetKey(window, GLFW_KEY_PAGE_DOWN);
    if (last_pgdown_state != pgdown_state && pgdown_state == GLFW_PRESS &&
        last_pgdown_state == GLFW_RELEASE) {
        if (config->render_distance > 1) { --(config->render_distance); }
    }
    last_pgdown_state = pgdown_state;

    static int last_f3_state = GLFW_RELEASE;
    const int f3_state       = glfwGetKey(window, GLFW_KEY_F3);
    if (last_f3_state != f3_state && f3_state == GLFW_PRESS &&
        last_f3_state == GLFW_RELEASE) {
        world_reload(world, config->render_distance);
    }
    last_f3_state = f3_state;
}

/**
 * @brief Managing inputs for mouse and keyboard. */
void handle_camera_mouse(GLFWwindow* window, const game_config_t* config,
                         const player_t* player, const float delta_time) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_move(player->camera, CAMERA_FORWARD, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_move(player->camera, CAMERA_BACKWARD, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_move(player->camera, CAMERA_LEFT, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_move(player->camera, CAMERA_RIGHT, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera_move(player->camera, CAMERA_UP, delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera_move(player->camera, CAMERA_DOWN, delta_time);
    }

    /* Speeding up when CTRL is pressed. */
    const int ctrl_state = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    if (ctrl_state == GLFW_PRESS) {
        player->camera->movement_speed = config->speed * 5;
    } else {
        player->camera->movement_speed = config->speed;
    }
}

int handle_clicks(GLFWwindow* window, world_t* world, const player_t* player,
                  vec3 target_block, vec3 neighbour, chunk_t* target_chunk,
                  chunk_t* neighbour_chunk) {
    static int last_lc_state = GLFW_RELEASE;
    static int last_rc_state = GLFW_RELEASE;
    static int memcheck      = 0;

    const int lc_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    const int rc_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    /* Left-click -> The block gets destroyed (replaced with air) */
    if (lc_state == GLFW_PRESS && last_lc_state == GLFW_RELEASE) {
        const int chunk_x = (int)floorf(target_block[0] / (float)CHUNK_SIZE_XZ);
        const int chunk_z = (int)floorf(target_block[2] / (float)CHUNK_SIZE_XZ);
        const int local_x = (int)floorf(target_block[0]) - (chunk_x * CHUNK_SIZE_XZ);
        const int local_y = (int)floorf(target_block[1]);
        const int local_z = (int)floorf(target_block[2]) - (chunk_z * CHUNK_SIZE_XZ);

        target_chunk->blocks[local_x][local_y][local_z] = (uint8_t)BLOCK_AIR;
        target_chunk->modified                          = true;
        world_rebuild_after_change(world, chunk_x, chunk_z, local_x, local_z);
    }

    /* Right-click -> A block is placed at the
     * neighbour coordinates. */
    if (rc_state == GLFW_PRESS && last_rc_state == GLFW_RELEASE) {
        if (!world_valid_position(world, neighbour)) { return 0; }

        const int chunk_x   = (int)floorf(neighbour[0] / (float)CHUNK_SIZE_XZ);
        const int chunk_z   = (int)floorf(neighbour[2] / (float)CHUNK_SIZE_XZ);
        const int n_local_x = (int)floorf(neighbour[0]) - (chunk_x * CHUNK_SIZE_XZ);
        const int n_local_y = (int)floorf(neighbour[1]);
        const int n_local_z = (int)floorf(neighbour[2]) - (chunk_z * CHUNK_SIZE_XZ);

        /* Player AABB */
        const float player_half_width = player->width / 2.0F;
        vec3 player_aabb[2]           = {{player->position[0] - player_half_width,
                                          player->position[1],
                                          player->position[2] - player_half_width},
                                         {player->position[0] + player_half_width,
                                          player->position[1] + player->height,
                                          player->position[2] + player_half_width}};

        /* Block AABB */
        vec3 block_aabb[2] = {
            {neighbour[0], neighbour[1], neighbour[2]},
            {neighbour[0] + 1.0F, neighbour[1] + 1.0F, neighbour[2] + 1.0F}};

        /* If player and future placed block interesect, do not place the block. */
        const bool overlap = glm_aabb_aabb(player_aabb, block_aabb);

        if (overlap) { return 0; }

        /* Can only place a block if there is air at the wanted spot */
        if (neighbour_chunk->blocks[n_local_x][n_local_y][n_local_z] !=
            (uint8_t)BLOCK_AIR) {
            return 0;
        }

        neighbour_chunk->blocks[n_local_x][n_local_y][n_local_z] =
            (uint8_t)BLOCK_COBBLESTONE;
        neighbour_chunk->modified = true;
        memcheck =
            world_rebuild_after_change(world, chunk_x, chunk_z, n_local_x, n_local_z);
    }

    last_lc_state = lc_state;
    last_rc_state = rc_state;
    if (memcheck < 0) { return -1; }
    return 0;
}

void handle_player_input(GLFWwindow* window, float* wish_forward, float* wish_right,
                         bool* jump_pressed, bool* sprint) {
    *wish_forward = 0.0F;
    *wish_right   = 0.0F;
    *jump_pressed = false;
    *sprint       = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { *wish_forward += 1.0F; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { *wish_forward -= 1.0F; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { *wish_right += 1.0F; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { *wish_right -= 1.0F; }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { *jump_pressed = true; }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { *sprint = true; }
}
