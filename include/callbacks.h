/**
 * @file callbacks.h
 * @authors Da-shou
 * @brief Header file containing all of the declrations of the functions passes as
 * callbacks
 * to the GLFW window.
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "opengl_glfw.h"

/**
 * @brief Function to pass as callback to GLFW window. Allows to use escape
 * to unfocus the window, as well as quit the game by pressing Q.
 * @param window Pointer to the window that will call the function
 * @param key The key integer that was pressed
 * @param scancode The scancode of the keypressed
 * @param action The action performed on the key
 * @param mode The current mode.
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

/**
 * @brief Function to pass as callback to GLFW window.
 * Allows to capture the movement of the mouse to move the camera.
 * @param window Pointer to the window that will call the function
 * @param x_pos The X position of the mouse on the window.
 * @param y_pos The Y position of the mouse on the window.
 */
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);

/**
 * @brief Function to pass as callback to GLFW window.
 * Allows to get the button clicks on the window. Used to focus the window and
 * render the mouse invisible.
 * @param window Pointer to the window that will call the function
 * @param button What click was pressed
 * @param action The action performed on the click
 * @param mods The current mode.
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

/**
 * @brief Function to pass as callback to GLFW window.
 * Called whenever the frame is resized.
 * @param window Pointer to the window that will call the function
 * @param width Width of the window once resized.
 * @param height Height of the window once resized.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#endif // CALLBACKS_H
