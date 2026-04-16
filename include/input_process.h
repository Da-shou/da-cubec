#ifndef INPUT_PROCESS_H
#define INPUT_PROCESS_H

#include <GLFW/glfw3.h>

#include "game_config.h"
#include "camera.h"
#include "world/chunk.h"
#include "world/world.h"

/**
 * @brief managing inputs for camera controls.
 * @param window Pointer to the window that will poll the mouse events.
 * @param config Game configuration to set the sensitivity.
 * @param camera Pointer to the camera that will be updated.
 */
void handle_camera_mouse(GLFWwindow* window, game_config_t* config,
                         camera_t* camera);

/**
 * @brief managing inputs for breaking and placing blocks.
 * @param window Pointer to the window that will poll the clicks.
 * @param world Pointer to the world that will be modified.
 * @param camera_pos Position of the camera.
 * @param target_block Position of the block that was clicked.
 * @param neighbour Position of the neighbour block.
 * @param target_chunk Pointer to the chunk that contains the target block.
 * @param neighbour_chunk Pointer to the chunk that contains the neighbour block.
 * @return 0 on success, -1 on chunk building failure.
 */
int handle_clicks(GLFWwindow* window, world_t* world, vec3 camera_pos, vec3 target_block,
                   vec3 neighbour, chunk_t* target_chunk,
                   chunk_t* neighbour_chunk);

#endif
