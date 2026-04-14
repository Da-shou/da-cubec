#ifndef INPUT_PROCESS_H
#define INPUT_PROCESS_H

#include <GLFW/glfw3.h>
#include <game_config.h>
#include <chunk.h>
#include <camera.h>
#include <world.h>

/**
 * @brief managing inputs for camera controls. */
void handle_camera_mouse(GLFWwindow* window, game_config_t* config,
                           camera_t* camera);

/**
 * @brief managing inputs for breaking and placing blocks. */
void handle_clicks(GLFWwindow* window, world_t* world,
                          vec3 target_block, vec3 neighbour,
                          chunk_t* target_chunk, chunk_t* neighbour_chunk);

#endif
