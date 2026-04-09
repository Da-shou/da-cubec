#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include "cglm/types.h"

typedef struct {
        vec3 up;
        vec3 right;
        vec3 front;
	vec3 target;
        vec3 direction;
        vec3 position;
} camera_t;

/** @brief Initializes the camera vectors and puts it at the
 * orgin of the world. 
 * @param camera Camera struct to be initalized. */
void camera_init(camera_t* camera);

/** @brief makes the camera look at a certain point in space.
 * @param camera Camera to be moved.
 * @param view View matrix to be modified. */
void camera_update(camera_t* camera, mat4 view);

/** @brief Allows the user to move the camera using keyboard and mouse
 * inputs. Has to be called every frame. 
 * @param window Window sending the inputs. 
 * @param camera Camera struct to be moved. */
void camera_process_inputs(GLFWwindow* window, camera_t* camera);

#endif
