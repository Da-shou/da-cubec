/**
 * @file main.h
 * @authors Da-shou
 * @brief Header file for the main source file containing most static initalization of
 * variables and signatures of pre-main loop helper functions and the main game loop
 * function as well.
 */

#ifndef MAIN_H
#define MAIN_H

/** Initial width of the window when launching the game, before applying the loaded
 * configuration */
#define INITIAL_WIDTH 800

/** Initial height of the window when launching the game, before applying the loaded
 * configuration */
#define INITIAL_HEIGHT 600

/** Initial title of the window when launching the game, before applying the loaded
 * configuration */
#define INITIAL_WINDOW_TITLE "da-cubec"

#include "game_state.h"

/** World structure that will contain all of the chunk and block infos */
static world_t* world;

/** Player structure to store all infos about the player in particular */
static player_t s_player;

/** First-person camera that will be linked to the player */
static camera_t s_main_camera;

/** Defining the static view matrix that will passed to the vertex shader */
static mat4 s_view_matrix;

/** Defining the static projection matrix that will passed to the vertex shader */
static mat4 s_projection_matrix;

/**
 * @brief Initalizes a new game state and returns it.
 *
 * @return Returns the static game state that will be used throughout.
 */
game_state_t game_state_init(void);

/**
 * @brief Main render loop of the game.
 *
 * @param game_window Pointer to GLFW window in which the game will be rendered.
 * @param state Pointer to the static game state containing information which all
 * functions inside the loop will need and share.
 */
void game_loop(GLFWwindow* game_window, game_state_t* state);

/**
 * @brief Shutdowns the game, frees all reserved memory and terminates GLFW.
 *
 * @param window Pointer to the window to destroy.
 * @param game_state Pointer to the game state to destroy.
 */
void game_shutdown(GLFWwindow* window, const game_state_t* game_state);

/**
 * @brief Initializes OpenGL/GLFW features needed to start the game.
 */
GLFWwindow* glfw_gl_init(int width, int height, const char* title);

#endif
