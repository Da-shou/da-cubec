#ifndef INPUT_PROCESS_H
#define INPUT_PROCESS_H

#include <GLFW/glfw3.h>

#include "game_config.h"
#include "camera.h"
#include "player.h"
#include "world/chunk.h"
#include "world/world.h"

/**
 * @brief managing inputs for camera controls.
 * @param window Pointer to the window that will poll the mouse events.
 * @param config Game configuration to set the sensitivity.
 * @param camera Pointer to the camera that will be updated.
 * @param delta_time delta time calculated in the main render loop
 */
void handle_camera_mouse(GLFWwindow* window, game_config_t* config, camera_t* camera,
                         float delta_time);

/**
 * @brief managing inputs for breaking and placing blocks.
 * @param window Pointer to the window that will poll the clicks.
 * @param world Pointer to the world that will be modified.
 * @param player Pointer to the player placing the blocks.
 * @param target_block Position of the block that was clicked.
 * @param neighbour Position of the neighbour block.
 * @param target_chunk Pointer to the chunk that contains the target block.
 * @param neighbour_chunk Pointer to the chunk that contains the neighbour block.
 * @return 0 on success, -1 on chunk building failure.
 */
int handle_clicks(GLFWwindow* window, world_t* world, const player_t* player,
                  vec3 target_block, vec3 neighbour, chunk_t* target_chunk,
                  chunk_t* neighbour_chunk);

/**
 * @brief Polls keyboard state and computes player movement wishes.
 * @param window    GLFW window to poll keys from.
 * @param config    Game config (for sprint speed).
 * @param[out] wish_forward  -1 (S), 0 (none), or +1 (W).
 * @param[out] wish_right    -1 (A), 0 (none), or +1 (D).
 * @param[out] jump_pressed  True if SPACE is pressed.
 * @param[out] sprint        True if CTRL is pressed.
 * @param[out] dt            Delta time for this frame.
 */
void handle_player_input(GLFWwindow* window, float* wish_forward, float* wish_right,
                         bool* jump_pressed, bool* sprint);

#endif
