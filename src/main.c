#include "cglm/cam.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cglm/cglm.h>

#include <shader.h>
#include <meshes/cube.h>

const uint16_t WIDTH = 800, HEIGHT = 600;
const char* const WINDOW_TITLE = "da-cubec";
const char* const VERTEX_SHADER_PATH = "src/shaders/basic.vert.glsl";
const char* const FRAGMENT_SHADER_PATH = "src/shaders/basic.frag.glsl";

mat4 view;
mat4 projection;

/**
 * @brief Called every time a key is pressed. */
void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
}

/**
 * @brief Called every time the window is resized */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
}

int main(void) {
        if (!glfwInit()) {
                fprintf(stderr, "%s\n",
                        "GLFW could not initialize. Exiting...");
                exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* Creating window */
        GLFWwindow* window =
            glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, NULL, NULL);
        if (window == NULL) {
                fprintf(stderr, "%s\n", "Failed to create GLFW window.");
                glfwTerminate();
                exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        /* Printing compilation and runtime infos */
        int version = gladLoadGL(glfwGetProcAddress);
        printf("\nNow launching application...\n");
        printf("Running on OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
               GLAD_VERSION_MINOR(version));
        printf("Compiled against GLFW %i.%i.%i\n", GLFW_VERSION_MAJOR,
               GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        printf("Running against GLFW %i.%i.%i\n", major, minor, revision);
        printf("Platform ID %d\n", glfwGetPlatform());

        /* Fix initial viewport using actual framebuffer size (may differ
         * from window size on HiDPI/Wayland displays) */
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);

        int width, height, nb_channels;
        unsigned char* image_data =
            stbi_load("img/stone.png", &width, &height, &nb_channels, 0);

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (!image_data) {
                fprintf(stderr, "%s\n",
                        "Failed to load texture image from disk");
        } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, image_data);
                glGenerateMipmap(GL_TEXTURE_2D);
        }

        stbi_image_free(image_data);

        /* Setting the texture parameters so that the texture repeats and
         * no linear interpolation is used to smooth out the textures. */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        shader_t basic_shader;
        shader_init(&basic_shader, VERTEX_SHADER_PATH,
                    FRAGMENT_SHADER_PATH);

        /* Set drawing mode (wireframe or full polygons) */
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        /* Enabling depth-testing so that OpenGL uses its Z-buffer to
        prioritze drawings vertices that are closer to the camera. */
        glEnable(GL_DEPTH_TEST);

        /* We then need a view matrix. To move around the world,
         * moving the camera is the same as moving the entire
         * world. Moving backwards = moving the entire scene
         * forward, etc. */
        glm_mat4_identity(view);
        glm_translate(view, (vec3) {0.0f, 0.0f, -10.0f});

        /* Last, we need a projection matrix to make the
         * perspective appear correctly. Since the calculation are
         * pretty complex, cglm provides us with the correct and
         * optimized functions*/
        glm_mat4_identity(projection);
        glm_perspective(glm_rad(45.0f), ((float)WIDTH / (float)HEIGHT),
                        0.1f, 100.0f, projection);

        int view_location = glGetUniformLocation(basic_shader.id, "view");

        int projection_location =
            glGetUniformLocation(basic_shader.id, "projection");

        cube_t cubes[25];
        int index;
        for (int i = 0; i < 5; ++i) {
                for (int j = 0; j < 5; ++j) {
                        index = i * 5 + j;
                        cube_init(&cubes[index]);
                        glm_vec3_copy((vec3) {i - 2, 0.0f, j - 2},
                                      cubes[index].position);
                        cube_update(&cubes[index]);
                }
        }

        /* Setting up the camera. It needs a position, a direction and a
         * target. The camera_direction vector is the substraction between
         * our position and the target that the camera needs to point at.
         * using basic vector math, this gets us the correct orentation.*/
        vec3 camera_position = {0.0f, 0.0f, 3.0f};
        vec3 camera_target = {0.0f, 0.0f, 0.0f};
        vec3 camera_direction;
        glm_vec3_sub(camera_position, camera_target, camera_direction);
        glm_normalize(camera_direction);

        /* We then need a right vector that represents the positive x-axis
         * of the camera. Using a cross product, we can get a perpendicular
         * vector from the plan made with an up vector and the camera
         * direction. */
        vec3 up = {0.0f, 1.0f, 0.0f};
        vec3 camera_right;
        glm_cross(up, camera_direction, camera_right);
        glm_normalize(camera_right);

        /* Finally, we calculate the up vector of the camera in the very
         * same way. Since both are normalized, no need to normalize again.
         */
        vec3 camera_up;
        glm_cross(camera_direction, camera_right, camera_up);

        /* Now, we can create a "look_at" matrix which will be useful in
         * Creating a camera. */
        glm_lookat((vec3) {0.0f, 0.0f, 3.0f}, (vec3) {0.0f, 0.0f, 0.0f},
                   (vec3) {0.0f, 1.0f, 0.0f}, view);

        const float radius = 10.0f;
        float cam_z, cam_x;

        /* Main window loop */
        while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                glClearColor(0.85f, 0.85f, 1.0f, 1.0f);

                /* When clearing, we need to clear the buffer bit and also
                 * the depth buffer
                 * bit so that information does not stack.
                 * We can use a bitwise OR to do both in one call. Very
                 * useful ! */
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glBindTexture(GL_TEXTURE_2D, texture);
                shader_use(&basic_shader);

                /* Make the camera rotate around the staircase */
                // glm_rotate(view, glm_rad((sin(glfwGetTime()))),
                //            (vec3) {0.0f, 1.0f, 0.0f});

                /* Draw all the cubes */
                for (int i = 0; i < 25; ++i) {
                        glm_vec3_copy(
                            (vec3) {cubes[i].position[0],
                                    sin(glfwGetTime() * 0.25f * i),
                                    cubes[i].position[2]},
                            cubes[i].position);
                        cube_update(&cubes[i]);
                        cube_draw(&cubes[i], &basic_shader);
                }

                cam_x = sin(glfwGetTime()) * radius;
                cam_z = cos(glfwGetTime()) * radius;
                glm_lookat((vec3) {cam_x, 0.0f, cam_z},
                           (vec3) {0.0f, 0.0f, 0.0f},
                           (vec3) {0.0f, 1.0f, 0.0f}, view);
                /* Apply the view and projection matrices */
                glUniformMatrix4fv(view_location, 1, GL_FALSE,
                                   (float*)view);

                glUniformMatrix4fv(projection_location, 1, GL_FALSE,
                                   (float*)projection);

                glfwSwapBuffers(window);
        }

        shader_destroy(&basic_shader);

        for (int i = 0; i < 25; ++i) { cube_free(&cubes[i]); }

        glfwDestroyWindow(window);
        glfwTerminate();

        printf("%s\n", "Exiting now...");
        return EXIT_SUCCESS;
}
