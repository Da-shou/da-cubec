#include "callbacks.h"
#include "game_state.h"
#include "camera.h"

void key_callback(GLFWwindow* window, const int key, const int scancode, const int action,
                  const int mode) {
    (void)scancode;
    (void)mode;
    game_state_t* const game_state = glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            game_state->focused = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera_reset_mouse();
        }
    } else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void mouse_callback(GLFWwindow* window, const double x_pos, const double y_pos) {
    (void)window;
    const game_state_t* const game_state = glfwGetWindowUserPointer(window);
    if (game_state->focused) {
        camera_rotate(game_state->main_camera, (float)x_pos, (float)y_pos, GL_TRUE);
    }
}

void mouse_button_callback(GLFWwindow* window, const int button, const int action,
                           const int mods) {
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT) { return; }
    game_state_t* const game_state = glfwGetWindowUserPointer(window);
    if (!game_state->focused && action == GLFW_PRESS) {
        game_state->focused = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    (void)window;
    glViewport(0, 0, width, height);
}