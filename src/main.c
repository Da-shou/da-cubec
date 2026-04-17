/* gl has to be included before GLFW */
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "shader.h"
#include "camera.h"
#include "game_config.h"
#include "input_process.h"
#include "material.h"
#include "player.h"
#include "text_renderer.h"
#include "world/blocks.h"
#include "world/chunk.h"
#include "world/pointer.h"
#include "world/world.h"

/* Defining the view and projection matrices. */
static mat4 view;
static mat4 projection;

static game_config_t config;

/* First-person camera */
static camera_t main_camera;

/* target_block is the block currently being pointed at by the player and
 * the one that will be destroyed if left click is pressed. */
static vec3 target_block;
static chunk_t* target_chunk;
/* Neighbour contains the coordinates of the block that will be placed if
 * right click is pressed. */
static vec3 neighbour;
static chunk_t* neighbour_chunk;

/* World structure that will contain all of the chunk and block infos */
static world_t world;
static player_t player;
static GLFWwindow* app_window;

/* If focused is true, the mouse is locked and camera movement is enabled.
 */
static bool focused = false;

/**
 * @brief Draws the name of the executable, version and GLFW/OpenGL version with a
 * text renderer.
 */
void draw_debug_info(const text_renderer_t* text_renderer);

/**
 * @brief Called every time a key is pressed. */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
/**
 * @brief Rotates camera if mouse is locked in the window. */
static void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);

/**
 * @brief Lock mouse if a left-clicked is performed on the window */
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
/**
 * @brief Called every time the window is resized */
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

/**
 * @brief Initializes OpenGL/GLFW features needed to start the game. */
static void glfw_gl_init();

static void reload_fog(const shader_t* shader);

int main(void) {
    config = game_config_default();

    glfw_gl_init();

    /* Creating our texture atlas */
    material_t atlas;
    material_create(&atlas, config.texture_atlas_path);

    /* Initalizing our shader */
    shader_t basic_shader;
    shader_init(&basic_shader, config.basic_vertex_shader_path,
                config.basic_fragment_shader_path);

    reload_fog(&basic_shader);

    /* Text renderer for the HUD debug overlay */
    static text_renderer_t debug_text_renderer;
    text_renderer_init(&debug_text_renderer, config.font_path,
                       config.text_vertex_shader_path, config.text_fragment_shader_path,
                       config.debug_font_size, config.width, config.height);

    camera_init(&config, &main_camera, (vec3) {0.0F, 127.0F, 0.0F});

    static perlin_params_t terrain = {0.01F, 64, 32};

    world_init(&world, &config);
    world.generate = world_generator_perlin;
    world.generator_data = &terrain;
    world_update(&world, GLM_VEC3_ZERO);

    /* We need a view matrix. To move around the world,
     * moving the camera is the same as moving the entire
     * world. Moving backwards = moving the entire scene
     * forward, etc. */
    glm_mat4_identity(view);
    glm_translate(view, GLM_VEC3_ZERO);

    /* Then, we need a projection matrix to make the
     * perspective appear correctly. Since the calculation are
     * pretty complex, cglm provides us with the correct and
     * optimized functions*/
    glm_mat4_identity(projection);
    glm_perspective(glm_rad(config.fov), ((float)config.width / (float)config.height),
                    0.1F, (float)(config.render_distance + 1) * CHUNK_SIZE_XZ,
                    projection);

    /* Getting the location of our uniform view and projection matrices
     * so that we can acces them in the render loop so we don't ask
     * OpenGL to give us the location each time. */
    const int view_location = glGetUniformLocation(basic_shader.id, "view");
    const int projection_location = glGetUniformLocation(basic_shader.id, "projection");

    /* Replace the camera spawn height with player spawn */
    /* The camera init position will be overwritten by player_update,
       but we still need it for initial vector setup */
    camera_init(&config, &main_camera, (vec3) {0.0F, 80.0F, 0.0F});

    /* Initalizing the global player variables */
    player_init(&player, &config, &main_camera, (vec3) {0.0F, 80.0F, 0.0F});
    static float wish_forward = 0.0F;
    static float wish_right = 0.0F;
    static float last_frame = 0.0F;
    static bool jump_pressed = false;
    static bool sprint = false;
    static vec3* pov_origin;
    static int last_render_distance = MAX_RENDER_DISTANCE;

    /* Main window loop */
    while (!glfwWindowShouldClose(app_window)) {
        /* Calling this function allows us to gather all inputs
         * from the user such as mouse or keyboard. */
        glfwPollEvents();

        /* Background color */
        glClearColor(config.sky_color[0], config.sky_color[1], config.sky_color[2],
                     config.sky_color[3]);

        /* When clearing, we need to clear the buffer bit and also
         * the depth buffer bit so that information does not stack.
         * We can use a bitwise OR to do both in one call. Very
         * useful ! */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        handle_debug_inputs(app_window, &config);

        const float current_frame = (float)glfwGetTime();
        const float delta_time = current_frame - last_frame;

        last_frame = current_frame;
        pov_origin = config.free_camera ? &main_camera.position : &player.position;

        /* Apply the view and projection matrices */
        shader_use(&basic_shader);
        glUniformMatrix4fv(view_location, 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (float*)projection);
        shader_set_vec3(&basic_shader, "camera_position", (float*)*pov_origin);

        if (config.render_distance != last_render_distance) {
            glm_perspective(
                glm_rad(config.fov), ((float)config.width / (float)config.height), 0.1F,
                (float)(config.render_distance + 1) * CHUNK_SIZE_XZ, projection);
            world_reload(&world, config.render_distance);
            reload_fog(&basic_shader);
        }
        last_render_distance = config.render_distance;

        /* Calculating the planes that make up the frustum of
         * the camera then culling everything that is outside the
         * frustum of the camera. Fortunately the CGLM libraries
         * has useful functions for this. */
        mat4 view_projection;
        glm_mat4_mul(projection, view, view_projection);
        vec4 frustum_planes[6];
        glm_frustum_planes(view_projection, frustum_planes);
        world_draw(&world, &basic_shader, &atlas, frustum_planes);

        /* Stream in/out chunks based on player position. */

        const int memcheck = world_update(&world, *pov_origin);
        if (memcheck < 0) {
            (void)fprintf(stderr,
                          "Memory allocation failure in world_update, exiting.\n");
            break;
        }

        camera_update_view(&main_camera, view);

        /* The freecam still updates the player's position so that the world
         * can keep loading. */
        if (config.free_camera) {
            handle_camera_mouse(app_window, &config, &player, delta_time);
            vec3 player_updated_position = {player.camera->position[0],
                                            player.camera->position[1] -
                                                player.eye_offset,
                                            player.camera->position[2]};
            glm_vec3_copy(GLM_VEC3_ZERO, player.velocity);
            glm_vec3_copy(player_updated_position, player.position);
        } else {
            handle_player_input(app_window, &wish_forward, &wish_right, &jump_pressed,
                                &sprint);
            player_update(&player, &config, &world, player.camera, wish_forward,
                          wish_right, jump_pressed, sprint, delta_time);

            const uint8_t block =
                get_pointed_block(&world, &main_camera, config.max_reach, &target_block,
                                  &neighbour, &target_chunk, &neighbour_chunk);
            if (block != (uint8_t)BLOCK_AIR) {
                if (handle_clicks(app_window, &world, &player, target_block, neighbour,
                                  target_chunk, neighbour_chunk)) {
                    (void)fprintf(stderr,
                                  "Chunk building failed after handle_click, exiting.\n");
                    break;
                };
            }
        }

        draw_debug_info(&debug_text_renderer);

        /* Swapping the buffers is a necessary step and I forgot
         * why. */
        glfwSwapBuffers(app_window);
    }

    shader_destroy(&basic_shader);
    text_renderer_destroy(&debug_text_renderer);
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
        (void)fprintf(stderr, "%s\n", "GLFW could not initialize. Exiting...");
        exit(EXIT_FAILURE);
    }

    app_window = glfwCreateWindow(config.width, config.height, config.title, NULL, NULL);
    if (app_window == NULL) {
        (void)fprintf(stderr, "%s\n", "Failed to create GLFW window.");
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
    printf("Compiled against GLFW %i.%i.%i\n", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR,
           GLFW_VERSION_REVISION);
    int major;
    int minor;
    int revision;
    glfwGetVersion(&major, &minor, &revision);
    printf("Running against GLFW %i.%i.%i\n", major, minor, revision);
    printf("Platform ID %d\n", glfwGetPlatform());

    /* Fix initial viewport using actual framebuffer size (may differ
     * from window size on HiDPI/Wayland displays) */
    int fb_width;
    int fb_height;
    glfwGetFramebufferSize(app_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    /* Set drawing mode (wireframe or full polygons) */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* Enabling depth-testing so that OpenGL uses its Z-buffer to
    prioritze drawings vertices that are closer to the camera. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void reload_fog(const shader_t* shader) {
    /* Setting our fog variables and sending them to the shader. The fog color will be the
     * same as the sky, fog_near is the distance from the camera from which the fog will
     * start, fog_far is when the fog will be at maximum density.*/
    shader_set_vec3(
        shader, "fog_color",
        (vec3) {config.sky_color[0], config.sky_color[1], config.sky_color[2]});
    const int max_render_distance = config.render_distance * CHUNK_SIZE_XZ;
    shader_set_int(shader, "max_render_distance", max_render_distance);
    shader_set_int(shader, "render_distance", config.render_distance);
}

void draw_debug_info(const text_renderer_t* text_renderer) {
    /* Draw game title in the bottom-left corner */

    static int version = 0;
    if (version == 0) { version = gladLoadGL(glfwGetProcAddress); }

    char title_text[64];
    (void)snprintf(title_text, sizeof(title_text), "%s %s", config.title, config.version);
    char opengl_info[64];
    (void)snprintf(opengl_info, sizeof(opengl_info), "GLFW %d.%d.%d OpenGL %d.%d",
                   GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION,
                   GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    char x_coordinates[64];
    (void)snprintf(x_coordinates, sizeof(x_coordinates), "X : %.3f", player.position[0]);
    char y_coordinates[64];
    (void)snprintf(y_coordinates, sizeof(y_coordinates), "Y : %.3f", player.position[1]);
    char z_coordinates[64];
    (void)snprintf(z_coordinates, sizeof(z_coordinates), "Z : %.3f", player.position[2]);
    char render_distance_text[64];
    (void)snprintf(render_distance_text, sizeof(render_distance_text),
                   "Render distance : %d", config.render_distance);
    char* coordinates[4] = {render_distance_text, x_coordinates, y_coordinates,
                            z_coordinates};

    const float line_spacing = text_renderer->font_size * 1.25F;
    const float margin_bottom = line_spacing - 20.0F;
    text_renderer_draw(text_renderer, title_text,
                       (vec2) {10.0F, (float)config.height - margin_bottom}, GLM_VEC3_ONE,
                       true, GLM_VEC3_ZERO, 0.33F);

    text_renderer_draw(
        text_renderer, opengl_info,
        (vec2) {10.0F, (float)config.height - margin_bottom - line_spacing}, GLM_VEC3_ONE,
        true, GLM_VEC3_ZERO, 0.33F);

    for (int8_t i = 0; i < 4; i++) {
        const float margin_top = 5.0F;
        text_renderer_draw(text_renderer, coordinates[i],
                           (vec2) {10.0F, text_renderer->font_size + margin_top +
                                              (line_spacing * (float)i)},
                           GLM_VEC3_ONE, true, GLM_VEC3_ZERO, 0.33F);
    }
}

void key_callback(GLFWwindow* window, const int key, const int scancode, const int action,
                  const int mode) {
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

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void mouse_callback(GLFWwindow* window, const double x_pos, const double y_pos) {
    (void)window;
    if (focused) { camera_rotate(&main_camera, (float)x_pos, (float)y_pos, GL_TRUE); }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void mouse_button_callback(GLFWwindow* window, const int button, const int action,
                           const int mods) {
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT) { return; }
    if (!focused && action == GLFW_PRESS) {
        focused = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    (void)window;
    glViewport(0, 0, width, height);
}
