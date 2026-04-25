#include "input_process.h"

#include <cglm/box.h>
#include <stdio.h>

#include "camera.h"
#include "world/blocks.h"
#include "world/block_actions.h"
#include "world/world.h"
#include "game_config.h"
#include "player.h"

void handle_debug_inputs(GLFWwindow* window, game_state_t* state) {
    static int last_tab_state = GLFW_RELEASE;
    const int tab_state       = glfwGetKey(window, GLFW_KEY_TAB);
    if (last_tab_state != tab_state && tab_state == GLFW_PRESS &&
        last_tab_state == GLFW_RELEASE) {
        state->config.free_camera = !state->config.free_camera;
    }
    last_tab_state = tab_state;

    static int last_pgup_state = GLFW_RELEASE;
    const int pgup_state       = glfwGetKey(window, GLFW_KEY_PAGE_UP);
    if (last_pgup_state != pgup_state && pgup_state == GLFW_PRESS &&
        last_pgup_state == GLFW_RELEASE) {
        if (state->config.render_distance < MAX_RENDER_DISTANCE) {
            ++(state->config.render_distance);
        }
    }
    last_pgup_state = pgup_state;

    static int last_pgdown_state = GLFW_RELEASE;
    const int pgdown_state       = glfwGetKey(window, GLFW_KEY_PAGE_DOWN);
    if (last_pgdown_state != pgdown_state && pgdown_state == GLFW_PRESS &&
        last_pgdown_state == GLFW_RELEASE) {
        if (state->config.render_distance > 1) { --(state->config.render_distance); }
    }
    last_pgdown_state = pgdown_state;

    static int last_f3_state = GLFW_RELEASE;
    const int f3_state       = glfwGetKey(window, GLFW_KEY_F3);
    if (last_f3_state != f3_state && f3_state == GLFW_PRESS &&
        last_f3_state == GLFW_RELEASE) {
        world_reload(state->world, state->config.render_distance);
        state->target_chunk    = NULL;
        state->neighbour_chunk = NULL;
    }
    last_f3_state = f3_state;
}

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
                  vec3 break_target, vec3 place_target) {
    static int last_lc_state = GLFW_RELEASE;
    static int last_rc_state = GLFW_RELEASE;
    static int memcheck      = 0;

    const int lc_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    const int rc_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    /* Left-click -> The block gets destroyed (replaced with air) */
    if (lc_state == GLFW_PRESS && last_lc_state == GLFW_RELEASE) {
        break_block(world, break_target);
    }

    /* Right-click -> A block is placed at the
     * neighbour coordinates. */
    if (rc_state == GLFW_PRESS && last_rc_state == GLFW_RELEASE) {
        place_block(world, player, place_target, player->block);
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

void handle_block_switch(GLFWwindow* window, block_type_t* block_type) {
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { *block_type = BLOCK_STONE; }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { *block_type = BLOCK_COBBLESTONE; }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { *block_type = BLOCK_DIRT; }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { *block_type = BLOCK_GRASS; }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { *block_type = BLOCK_SAND; }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { *block_type = BLOCK_LAMP; }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { *block_type = BLOCK_PLANKS; }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { *block_type = BLOCK_LOG; }
}
