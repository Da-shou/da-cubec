#include "cglm/cam.h"
#include "cglm/vec3.h"
#include <camera.h>

#include <GLFW/glfw3.h>

static vec3 UP = {0.0f, 1.0f, 0.0f};

void camera_init(camera_t* camera) {
        /* Setting up the camera. It needs a position, a direction and a
         * target. The camera_direction vector is the substraction between
         * our position and the target that the camera needs to point at.
         * using basic vector math, this gets us the correct orentation.*/
        glm_vec3_copy((vec3) {0.0f, 0.0f, 3.0f}, camera->position);
        glm_vec3_copy(GLM_VEC3_ZERO, camera->target);
        glm_vec3_copy((vec3) {0.0f, 0.0f, -1.0f}, camera->front);
        glm_vec3_sub(camera->position, camera->target, camera->direction);
        glm_normalize(camera->direction);

        /* We then need a right vector that represents the positive x-axis
         * of the camera. Using a cross product, we can get a perpendicular
         * vector from the plan made with an up vector and the camera
         * direction. */
        glm_cross(UP, camera->direction, camera->right);
        glm_normalize(camera->right);

        /* Finally, we calculate the up vector of the camera in the very
         * same way. Since both are normalized, no need to normalize again.
         */
        glm_cross(camera->direction, camera->right, camera->up);
}

void camera_update(camera_t* camera, mat4 view) {
        vec3 direction;
        glm_vec3_add(camera->position, camera->front, direction);
        glm_lookat(camera->position, direction, camera->up, view);
}

void camera_process_inputs(GLFWwindow* window, camera_t* camera) {
	float delta_time = 0.0f;
	float last_frame = 0.0f;
	float current_frame = glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;

        const float camera_speed = 0.05f * delta_time;

	vec3 temp;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		glm_vec3_scale(camera->front, camera_speed, temp);
                glm_vec3_add(camera->position, temp, camera->position);
	}
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {	
		glm_vec3_scale(camera->front, camera_speed, temp);
                glm_vec3_sub(camera->position, temp, camera->position);
	}
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		glm_cross(camera->front, camera->up, temp);
		glm_normalize(temp);
		glm_vec3_scale(temp, camera_speed, temp);
		glm_vec3_sub(camera->position, temp, camera->position);
	}
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {	
		glm_cross(camera->front, camera->up, temp);
		glm_normalize(temp);
		glm_vec3_scale(temp, camera_speed, temp);
		glm_vec3_add(camera->position, temp, camera->position);
	}
}
