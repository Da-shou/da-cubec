#include "shader_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

const uint16_t WIDTH = 800, HEIGHT = 600;
const char* const WINDOW_TITLE = "da-cubec";
const char* VERTEX_SHADER_PATH = "src/shaders/basic.vert.glsl";
const char* FRAGMENT_SHADER_PATH = "src/shaders/basic.frag.glsl";

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
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

        // Creating window
        GLFWwindow* window =
            glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, NULL, NULL);
        glfwMakeContextCurrent(window);

        glfwSetKeyCallback(window, key_callback);

        // Printing compilation and runtime infos
        int version = gladLoadGL(glfwGetProcAddress);
        printf("Running on OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
               GLAD_VERSION_MINOR(version));
        printf("Compiled against GLFW %i.%i.%i\n", GLFW_VERSION_MAJOR,
               GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        printf("Running against GLFW %i.%i.%i\n", major, minor, revision);
        printf("Platform ID %d\n", glfwGetPlatform());

        // Setting up the triangle
        float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                            0.0f,  0.0f,  0.5f, 0.0f};

        unsigned int shader_program =
            make_shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        // Main window loop
        while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(0.5f, 0.6f, 0.9f, 1.0f);
                glUseProgram(shader_program);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
                glfwSwapBuffers(window);
        }

        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_SUCCESS;
}
