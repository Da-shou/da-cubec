#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <shader.h>
#include <camera.h>
#include <material.h>
#include <blocks.h>
#include <chunk.h>
#include <pointer.h>
#include <game_config.h>

const uint16_t WIDTH = 800, HEIGHT = 600;
const char* const WINDOW_TITLE = "da-cubec";
const char* const VERTEX_SHADER_PATH = "src/shaders/basic.vert.glsl";
const char* const FRAGMENT_SHADER_PATH = "src/shaders/basic.frag.glsl";
const char* const TEXTURE_ATLAS_PATH = "img/atlas.png";
const float_t MAX_REACH = 6.0f;

mat4 view;
mat4 projection;
camera_t main_camera;

/* target is the block currently being pointed at by the player and the one
 * that will be destroyed if left click is pressed. */
vec3 target_block;
chunk_t* target_chunk;

/* neighbour contains the coordinates of the block that will be placed if
 * right click is pressed. */
vec3 neighbour;

world_t world;

float delta_time = 0.0f;
float last_frame = 0.0f;

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
 * @brief Managing inputs for camera controls. */
void process_camera_inputs(GLFWwindow* window, camera_t* camera);

/**
 * @brief Managing inputs for breaking and placing blocks. */
void process_block_inputs(GLFWwindow* window);

/**
 * @brief Called every time the window is resized */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(void) {
        if (!glfwInit()) {
                fprintf(stderr, "%s\n",
                        "GLFW could not initialize. Exiting...");
                exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        game_config_t config = game_config_default();

        /* Creating window */
        GLFWwindow* window =
            glfwCreateWindow(config.width, config.height, config.title, NULL, NULL);
        if (window == NULL) {
                fprintf(stderr, "%s\n", "Failed to create GLFW window.");
                glfwTerminate();
                exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);

        /* Setting all of our callbacks for various events such as moving
         * the cursor, clicking on the window, or resizing the window.*/
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);

        /* Creating our texture atlas */
        material_t atlas;
        material_create(&atlas, config.texture_atlas_path);

        /* Initalizing our shader */
        shader_t basic_shader;
        shader_init(&basic_shader, config.vertex_shader_path,
                    config.fragment_shader_path);

        camera_init(&config, &main_camera, (vec3) {
                (float)WORLD_SIZE_X * CHUNK_SIZE_XZ / 2.0f,
                20.f,
                (float)WORLD_SIZE_X * CHUNK_SIZE_XZ / 2.0f
        });

        world_init(&world);
        world_fill_superflat(&world);
        world_build(&world);

        /* Set drawing mode (wireframe or full polygons) */
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        /* Enabling depth-testing so that OpenGL uses its Z-buffer to
        prioritze drawings vertices that are closer to the camera. */
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

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
        glm_perspective(glm_rad(70.0f), ((float)WIDTH / (float)HEIGHT),
                        0.1f, 100.0f, projection);

        /* Getting the location of our uniform view and projection matrices
         * so that we can acces them in the render loop so we don't ask
         * OpenGL to give us the location each time. */
        const int view_location = glGetUniformLocation(basic_shader.id, "view");
        const int projection_location =
            glGetUniformLocation(basic_shader.id, "projection");

        /* Main window loop */
        while (!glfwWindowShouldClose(window)) {
                /* Calling this function allows us to gather all inputs
                 * from the user such as mouse or keyboard. */
                glfwPollEvents();

                /* Background color */
                glClearColor(0.85f, 0.85f, 1.0f, 1.0f);

                /* When clearing, we need to clear the buffer bit and also
                 * the depth buffer bit so that information does not stack.
                 * We can use a bitwise OR to do both in one call. Very
                 * useful ! */
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                process_camera_inputs(window, &main_camera);
                camera_update_view(&main_camera, view);
                if (focused) {
                        const block_type_t block = get_pointed_block(
                                &world, &main_camera, MAX_REACH,
                                &target_block, &neighbour, &target_chunk);
                        if (block != BLOCK_AIR) {
                                process_block_inputs(window);
                        }
                }

                /* Apply the view and projection matrices */
                glUniformMatrix4fv(view_location, 1, GL_FALSE,
                                   (float*)view);
                glUniformMatrix4fv(projection_location, 1, GL_FALSE,
                                   (float*)projection);

                world_draw(&world, &basic_shader, &atlas);

                /* Swapping the buffers is a necessary step and I forgot
                 * why. */
                glfwSwapBuffers(window);
        }

        shader_destroy(&basic_shader);
        material_destroy(&atlas);
        world_destroy(&world);

        glfwDestroyWindow(window);
        glfwTerminate();

        printf("%s\n", "Exiting now...");
        return EXIT_SUCCESS;
}

/**
 * @brief Called every time a key is pressed. */
void key_callback(GLFWwindow* window, const int key, const int scancode, const int action,
                  const int mode) {
        (void)scancode;
        (void)mode;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                const int current = glfwGetInputMode(window, GLFW_CURSOR);
                if (current == GLFW_CURSOR_DISABLED) {
                        focused = false;
                        glfwSetInputMode(window, GLFW_CURSOR,
                                         GLFW_CURSOR_NORMAL);
                        camera_reset_mouse();
                }
        } else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GL_TRUE);
        }
}

/**
 * @brief Rotates camera if mouse is locked in the window. */
void mouse_callback(GLFWwindow* window, const double x_pos,
                    const double y_pos) {
        (void)window;
        if (focused) {
                camera_rotate(&main_camera, (float)x_pos, (float)y_pos,
                              GL_TRUE);
        }
}

void mouse_button_callback(GLFWwindow* window, const int button, const int action,
                           const int mods) {
        (void)mods;
        if (button != GLFW_MOUSE_BUTTON_LEFT) return;
        if (!focused && action == GLFW_PRESS) {
                focused = true;
                glfwSetInputMode(window, GLFW_CURSOR,
                                 GLFW_CURSOR_DISABLED);
        }
}

/**
 * @brief Managing inputs for mouse and keyboard. */
void process_camera_inputs(GLFWwindow* window, camera_t* camera) {
        const float current_frame = (float)glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                camera_move(camera, CAMERA_FORWARD, delta_time);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                camera_move(camera, CAMERA_BACKWARD, delta_time);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                camera_move(camera, CAMERA_LEFT, delta_time);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                camera_move(camera, CAMERA_RIGHT, delta_time);
        }
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera_move(camera, CAMERA_UP, delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera_move(camera, CAMERA_DOWN, delta_time);
	}
}

void process_block_inputs(GLFWwindow* window) {
        static int last_lc_state = GLFW_RELEASE;
        static int last_rc_state = GLFW_RELEASE;

        const int lc_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        const int rc_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

        /* Left-click -> The block gets destroyed (replaced with air) */
        if (lc_state == GLFW_PRESS && last_lc_state == GLFW_RELEASE) {
                const int lx = (int)target_block[0] % CHUNK_SIZE_XZ;
                const int ly = (int)target_block[1];
                const int lz = (int)target_block[2] % CHUNK_SIZE_XZ;
                const int cx = (int)target_block[0] / CHUNK_SIZE_XZ;
                const int cz = (int)target_block[2] / CHUNK_SIZE_XZ;

                /** Destroying the block that is being looked at **/
                target_chunk->blocks[lx][ly][lz] = BLOCK_AIR;
                world_rebuild_after_change(&world, cx, cz, lx, lz);
        }

        /* Right-click -> A block is placed at the neighbour coordinates. */
        if (rc_state == GLFW_PRESS && last_rc_state == GLFW_RELEASE) {
                if (!world_valid_position(neighbour)) return;
                const int lx = (int)neighbour[0] % CHUNK_SIZE_XZ;
                const int ly = (int)neighbour[1];
                const int lz = (int)neighbour[2] % CHUNK_SIZE_XZ;
                const int cx = (int)neighbour[0] / CHUNK_SIZE_XZ;
                const int cz = (int)neighbour[2] / CHUNK_SIZE_XZ;
                if (target_chunk->blocks[lx][ly][lz] != BLOCK_AIR) return;
                target_chunk->blocks[lx][ly][lz] = BLOCK_COBBLESTONE;
                world_rebuild_after_change(&world, cx, cz, lx, lz);
        }

	last_lc_state = lc_state;
	last_rc_state = rc_state;
}

/**
 * @brief Called every time the window is resized */
void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
        (void)window;
        glViewport(0, 0, width, height);
}
