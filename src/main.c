#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <shader.h>

const uint16_t WIDTH = 800, HEIGHT = 600;
const char* const WINDOW_TITLE = "da-cubec";
const char* const VERTEX_SHADER_PATH = "src/shaders/basic.vert.glsl";
const char* const FRAGMENT_SHADER_PATH = "src/shaders/basic.frag.glsl";

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
}

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

        /* Setting up the vertices used by the triangles */
        float vertices[] = {
            // positions	// colors
            0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // top right
            0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, // top left
            -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f  // bottom left
        };

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
        unsigned int indices[] = {0, 1, 3, 0, 2, 3};

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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Set drawing mode (wireframe or full polygons)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Main window loop
        while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                glClearColor(0.85f, 0.85f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                shader_use(&basic_shader);

                glBindVertexArray(VAO);

                // Draw triangles
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
