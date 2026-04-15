#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* gl has to be included before GLFW */
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "shader.h"
#include "camera.h"
#include "material.h"
#include "blocks.h"
#include "chunk.h"
#include "pointer.h"
#include "world.h"
#include "game_config.h"
#include "input_process.h"

/* Defining the view and projection matrices. */
mat4 view;
mat4 projection;

game_config_t config;

/* First-person camera */
camera_t main_camera;

/* target_block is the block currently being pointed at by the player and
 * the one that will be destroyed if left click is pressed. */
vec3 target_block;
chunk_t* target_chunk;
/* Neighbour contains the coordinates of the block that will be placed if
 * right click is pressed. */
vec3 neighbour;
chunk_t* neighbour_chunk;

/* World structure that will contain all of the chunk and block infos */
world_t world;

GLFWwindow* app_window;

/* If focused is true, the mouse is locked and camera movement is enabled.
 */
static bool focused = false;

/**
 * @brief Called every time a key is pressed. */
void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode);
/**
 * @brief Rotates camera if mouse is locked in the window. */
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);

/**
 * @brief Lock mouse if a left-clicked is performed on the window */
void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods);
/**
 * @brief Called every time the window is resized */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

/**
 * @brief Initializes OpenGL/GLFW features needed to start the game. */
void glfw_gl_init();

int main(void) {
    config = game_config_default();

    glfw_gl_init();

    /* Creating our texture atlas */
    material_t atlas;
    material_create(&atlas, config.texture_atlas_path);

    /* Initalizing our shader */
    shader_t basic_shader;
    shader_init(&basic_shader, config.vertex_shader_path,
                config.fragment_shader_path);

    camera_init(&config, &main_camera,
                (vec3) {(float)CHUNK_SIZE_XZ / 2.0f, 128.0f,
                        (float)CHUNK_SIZE_XZ / 2.0f});

    static perlin_params_t terrain = {0.01f, 64, 32};

    world_init(&world, &config);
    world.generate = world_generator_perlin;
    world.generator_data = &terrain;

    /* We need a view matrix. To move around the world,
     * moving the camera is the same as moving the entire
     * world. Moving backwards = moving the entire scene
     * forward, etc. */
    glm_mat4_identity(view);
    glm_translate(view, (vec3) {0.0f, 0.0f, 0.0f});

    /* Then, we need a projection matrix to make the
     * perspective appear correctly. Since the calculation are
     * pretty complex, cglm provides us with the correct and
     * optimized functions*/
    glm_mat4_identity(projection);
    glm_perspective(glm_rad(70.0f),
                    ((float)config.width / (float)config.height), 0.1f,
                    (float)(config.render_distance + 1) * CHUNK_SIZE_XZ, projection);

    /* Getting the location of our uniform view and projection matrices
     * so that we can acces them in the render loop so we don't ask
     * OpenGL to give us the location each time. */
    const int view_location =
        glGetUniformLocation(basic_shader.id, "view");
    const int projection_location =
        glGetUniformLocation(basic_shader.id, "projection");

    /* Main window loop */
    while (!glfwWindowShouldClose(app_window)) {
        /* Calling this function allows us to gather all inputs
         * from the user such as mouse or keyboard. */
        glfwPollEvents();

        /* Stream in/out chunks based on player position. */
        world_update(&world, main_camera.position);

        /* Background color */
        glClearColor(0.85f, 0.85f, 1.0f, 1.0f);

        /* When clearing, we need to clear the buffer bit and also
         * the depth buffer bit so that information does not stack.
         * We can use a bitwise OR to do both in one call. Very
         * useful ! */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        handle_camera_mouse(app_window, &config, &main_camera);
        camera_update_view(&main_camera, view);
        if (focused) {
            const uint8_t block = get_pointed_block(
                &world, &main_camera, config.max_reach, &target_block,
                &neighbour, &target_chunk, &neighbour_chunk);
            if (block != (uint8_t)BLOCK_AIR) {
                handle_clicks(app_window, &world, target_block, neighbour,
                              target_chunk, neighbour_chunk);
            }
        }

        /* Apply the view and projection matrices */
        glUniformMatrix4fv(view_location, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projection_location, 1, GL_FALSE,
                           (float*)projection);

        /* Calculating the planes that make up the frustum of
         * the camera then culling everything that is outside the
         * frustum of the camera. Fortunately the CGLM libraries
         * has useful functions for this. */
        mat4 vp;
        glm_mat4_mul(projection, view, vp);
        vec4 frustum_planes[6];
        glm_frustum_planes(vp, frustum_planes);
        world_draw(&world, &basic_shader, &atlas, frustum_planes);

        /* Swapping the buffers is a necessary step and I forgot
         * why. */
        glfwSwapBuffers(app_window);
    }

    shader_destroy(&basic_shader);
    material_destroy(&atlas);
    world_destroy(&world);

    glfwDestroyWindow(app_window);
    glfwTerminate();

    printf("%s\n", "Exiting now...");
    return EXIT_SUCCESS;
}

void glfw_gl_init() {
    /* Creating window */
    if (!glfwInit()) {
        fprintf(stderr, "%s\n", "GLFW could not initialize. Exiting...");
        exit(EXIT_FAILURE);
    }

    app_window = glfwCreateWindow(config.width, config.height, config.title,
                              NULL, NULL);
    if (app_window == NULL) {
        fprintf(stderr, "%s\n", "Failed to create GLFW window.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(app_window);
    /* Setting all of our callbacks for various events such as moving
     * the cursor, clicking on the window, or resizing the window.*/
    glfwSetKeyCallback(app_window, key_callback);
    glfwSetCursorPosCallback(app_window, mouse_callback);
    glfwSetMouseButtonCallback(app_window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(app_window, framebuffer_size_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Printing compilation and runtime infos */
    const int version = gladLoadGL(glfwGetProcAddress);
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
    glfwGetFramebufferSize(app_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    /* Set drawing mode (wireframe or full polygons) */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* Enabling depth-testing so that OpenGL uses its Z-buffer to
    prioritze drawings vertices that are closer to the camera. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void key_callback(GLFWwindow* window, const int key, const int scancode,
                  const int action, const int mode) {
    (void)scancode;
    (void)mode;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        const int current = glfwGetInputMode(window, GLFW_CURSOR);
        if (current == GLFW_CURSOR_DISABLED) {
            focused = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera_reset_mouse();
        }
    } else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void mouse_callback(GLFWwindow* window, const double x_pos,
                    const double y_pos) {
    (void)window;
    if (focused) {
        camera_rotate(&main_camera, (float)x_pos, (float)y_pos, GL_TRUE);
    }
}

void mouse_button_callback(GLFWwindow* window, const int button,
                           const int action, const int mods) {
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (!focused && action == GLFW_PRESS) {
        focused = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void framebuffer_size_callback(GLFWwindow* window, const int width,
                               const int height) {
    (void)window;
    glViewport(0, 0, width, height);
}
