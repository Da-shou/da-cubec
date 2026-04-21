#ifndef MAIN_H
#define MAIN_H

#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600
#define INITIAL_WINDOW_TITLE "da-cubec"

#include "game_state.h"

/* World structure that will contain all of the chunk and block infos */
static world_t s_world;
static player_t s_player;

/* First-person camera */
static camera_t s_main_camera;

/* Defining the view and projection matrices. */
static mat4 s_view_matrix;
static mat4 s_projection_matrix;

/**
 * @brief Initalizes a new game state and returns it.
 *
 * @return Returns the static game state that will be used throughout.
 */
static game_state_t game_state_init(void);

/**
 * @brief Main render loop of the game.
 *
 * @param game_window Pointer to GLFW window in which the game will be rendered.
 * @param state Pointer to the static game state containing information which all
 * functions inside the loop will need and share.
 */
static void game_loop(GLFWwindow* game_window, game_state_t* state);

/**
 * @brief Shutdowns the game, frees all reserved memory and terminates GLFW.
 *
 * @param window Pointer to the window to destroy.
 * @param game_state Pointer to the game state to destroy.
 */
static void game_shutdown(GLFWwindow* window, const game_state_t* game_state);

/**
 * @brief Initializes OpenGL/GLFW features needed to start the game.
 */
GLFWwindow* glfw_gl_init(int width, int height, const char* title);

#endif
