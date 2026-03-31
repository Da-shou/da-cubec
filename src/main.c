#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

const uint16_t WIDTH = 800, HEIGHT = 600;
const char* const WINDOW_TITLE = "da-cubec";

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void) {
        if (!glfwInit()) {
		fprintf(stderr, "%s\n", "GLFW could not initialize. Exiting...");
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

        // Main window loop
        while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                glClearColor(0.7f, 0.7f, 0.9f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                glfwSwapBuffers(window);
        }

	glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_SUCCESS;
}
