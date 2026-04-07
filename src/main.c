#include "cglm/mat4.h"
#include "cglm/util.h"
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

	cube_t cube;	
	cube_init(&cube);
	
        /* Set drawing mode (wireframe or full polygons) */
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        /* Enabling depth-testing so that OpenGL uses its Z-buffer to
        prioritze drawings vertices that are closer to the camera. */
        glEnable(GL_DEPTH_TEST);

        /* Main window loop */
        while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                glClearColor(0.85f, 0.85f, 1.0f, 1.0f);

                /* When clearing, we need to clear the buffer bit and also
                 * the depth buffer bit so that information does not stack.
                 * We can use a bitwise OR to do both in one call. Very
                 * useful ! */
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glBindTexture(GL_TEXTURE_2D, texture);
                shader_use(&basic_shader);

                /* To go 3D, we will first need a model matrix. We will
                 * transform our plane to make it look like it lays on the
                 * floor. We will do this by rotating it 55 degrees on the
                 * x axis. */
                mat4 model;
                glm_mat4_identity(model);
                glm_rotate(model,
                           (float)glfwGetTime() * 2.0f * glm_rad(-55.0f),
                           (vec3) {1.0f, 0.0f, 1.0f});

                /* We then need a view matrix. To move around the world,
                 * moving the camera is the same as moving the entire
                 * world. Moving backwards = moving the entire scene
                 * forward, etc. */
                mat4 view;
                glm_mat4_identity(view);
                glm_translate(view, (vec3) {0.0f, 0.0f, -3.0f});

                /* Last, we need a projection matrix to make the
                 * perspective appear correctly. Since the calculation are
                 * pretty complex, cglm provides us with the correct and
                 * optimized functions*/
                mat4 projection;
                glm_mat4_identity(projection);
                glm_perspective(glm_rad(45.0f),
                                ((float)WIDTH / (float)HEIGHT), 0.1f,
                                100.0f, projection);

                /* Sending matrices to the shader every frame */
                int model_location =
                    glGetUniformLocation(basic_shader.id, "model");
                glUniformMatrix4fv(model_location, 1, GL_FALSE,
                                   (float*)model);

                int view_location =
                    glGetUniformLocation(basic_shader.id, "view");
                glUniformMatrix4fv(view_location, 1, GL_FALSE,
                                   (float*)view);

                int projection_location =
                    glGetUniformLocation(basic_shader.id, "projection");
                glUniformMatrix4fv(projection_location, 1, GL_FALSE,
                                   (float*)projection);	

		//cube_draw(&cube);

                glfwSwapBuffers(window);
        }

        shader_destroy(&basic_shader);
	cube_free(&cube);	

        glfwDestroyWindow(window);
        glfwTerminate();

        printf("%s\n", "Exiting now...");
        return EXIT_SUCCESS;
}
