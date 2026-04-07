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

        /* These are the vertices for the a textured cube. There are 36
         * vertices here.*/
        float vertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
            0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f,
            1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  -0.5f, -0.5f,
            -0.5f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  0.5f,
            -0.5f, 0.5f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,
            1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
            1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 1.0f,  1.0f,  -0.5f, -0.5f,
            -0.5f, 0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  -0.5f,
            -0.5f, 0.5f,  0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,
            1.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  1.0f,  0.5f,  -0.5f, -0.5f,
            0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  0.5f,  0.5f,
            0.5f,  1.0f,  0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  0.5f,
            -0.5f, -0.5f, 1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,
            0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,
            0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  -0.5f, 0.5f,  -0.5f,
            0.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  1.0f,  0.5f,  0.5f,
            0.5f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  -0.5f,
            0.5f,  0.5f,  0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f};

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

        shader basic_shader;
        shader_init(&basic_shader, VERTEX_SHADER_PATH,
                    FRAGMENT_SHADER_PATH);

        unsigned int VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        /* Generate a vertex buffer and put its ID in VBO. The buffer type
         * of vertex buffers is GL_ARRAY_BUFFER */
        unsigned int VBO;
        glGenBuffers(1, &VBO);

        /* Bind our new vertex buffer so that any change to the OpenGL
         * buffer are done to the currenlty bound buffer. */
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        /* Copy some user data into the currenlty bound buffer
         * We use GL_STATIC_DRAW as the triangle will not move for now. */
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);

        /* Creating an element array buffer to store sets of vertices
         * to be reused. In this case, to draw two triangles. */
        unsigned int EBO;
        glGenBuffers(1, &EBO);

        /* Defining our two triangles (0,1,3) and (1,2,3) */
        unsigned int indices[] = {2, 3, 1, 0, 1, 3};

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                     GL_STATIC_DRAW);

        /* This is the important part, we are defining where each
         * attributes of our vertices are located in the vertices array. We
         * have a location, defined by three floats and a color, also
         * defined by three floats. We thus have two parameters, 0 for
         * position and 1 for color. The position parameter starts with an
         * offset of 0 and has to skip 6*sizeof(float) to get to the next
         * parameter. This space is called the stride. The color parameter
         * starts with an offset of 3*sizeof(float) and has the same stride
         */
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);
        /* Also informing that the vertex has a new attribute, its texture
         * coordinates.*/
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

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
                glBindVertexArray(VAO);

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

                // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Unbind VAO for safety.
                glBindVertexArray(0);
                glfwSwapBuffers(window);
        }

        shader_destroy(&basic_shader);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);

        glfwDestroyWindow(window);
        glfwTerminate();

        printf("%s\n", "Exiting now...");
        return EXIT_SUCCESS;
}
