#include <GLFW/glfw3.h>

#include "callbacks.h"
#include "game_context.h"
#include "camera.h"

void key_callback(GLFWwindow* window, const int key, const int scancode, const int action,
                  const int mode) {
    (void)scancode;
    (void)mode;
    game_context_t* const ctx = get_game_context();

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            ctx->focused = false;
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
    const game_context_t* const ctx = get_game_context();
    if (ctx->focused) {
        camera_rotate(ctx->main_camera, (float)x_pos, (float)y_pos, GL_TRUE);
    }
}

void mouse_button_callback(GLFWwindow* window, const int button, const int action,
                           const int mods) {
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT) { return; }
    game_context_t* const ctx = get_game_context();
    if (!ctx->focused && action == GLFW_PRESS) {
        ctx->focused = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void framebuffer_size_callback(GLFWwindow* window, const int width,
                               const int height) {
    (void)window;
    glViewport(0, 0, width, height);
}