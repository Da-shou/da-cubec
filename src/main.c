/* gl has to be included before GLFW */
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "shader.h"
#include "camera.h"
#include "game_config.h"
#include "game_state.h"
#include "callbacks.h"
#include "input_process.h"
#include "material.h"
#include "player.h"
#include "text_renderer.h"
#include "world/blocks.h"
#include "world/chunk.h"
#include "world/fog.h"
#include "world/pointer.h"
#include "world/world.h"

/* World structure that will contain all of the chunk and block infos */
static world_t s_world;
static player_t s_player;

/* First-person camera */
static camera_t s_main_camera;

/* Defining the view and projection matrices. */
static mat4 s_view_matrix;
static mat4 s_projection_matrix;

/**
 * @brief Draws the name of the executable, version and GLFW/OpenGL version with a
 * text renderer.
 */
void draw_debug_info(const text_renderer_t* text_renderer,
                     const game_state_t* game_state);

static game_state_t game_state_init(void);

static void game_loop(GLFWwindow* game_window, game_state_t* state);
static void game_shutdown(GLFWwindow* window, const game_state_t* game_state);

/**
 * @brief Initializes OpenGL/GLFW features needed to start the game. */
GLFWwindow* glfw_gl_init(int width, int height, const char* title);

int main(void) {
    const game_config_t preconfig = game_config_default();
    GLFWwindow* game_window =
        glfw_gl_init(preconfig.width, preconfig.height, preconfig.title);
    game_state_t state = game_state_init();
    glfwSetWindowUserPointer(game_window, &state);
    game_loop(game_window, &state);
    game_shutdown(game_window, &state);
    return EXIT_SUCCESS;
}

static game_state_t game_state_init(void) {
    game_state_t state = {0};

    state.config = game_config_default();

    // Point to file-scope structs for large objects
    state.world = &s_world;
    state.player = &s_player;
    state.main_camera = &s_main_camera;

    // Shaders and assets
    shader_init(&state.cube_shader, state.config.basic_vertex_shader_path,
                state.config.basic_fragment_shader_path);
    material_create(&state.atlas, state.config.texture_atlas_path);
    reload_fog(&state);

    // Text renderer
    text_renderer_init(
        &state.debug_text_renderer, state.config.font_path,
        state.config.text_vertex_shader_path, state.config.text_fragment_shader_path,
        state.config.debug_font_size, state.config.width, state.config.height);

    // Camera — initialized twice intentionally, second sets actual spawn
    camera_init(&state.config, &s_main_camera, (vec3) {0.0F, 127.0F, 0.0F});
    camera_init(&state.config, &s_main_camera, (vec3) {0.0F, 80.0F, 0.0F});

    // Player
    player_init(state.player, &state.config, &s_main_camera, (vec3) {0.0F, 80.0F, 0.0F});

    // World
    static perlin_params_t terrain = {0.01F, 64, 32};
    world_init(state.world, &state.config);
    state.world->generate = world_generator_perlin;
    state.world->generator_data = &terrain;
    world_update(state.world, GLM_VEC3_ZERO);

    // Matrices
    glm_mat4_identity(s_view_matrix);
    glm_translate(s_view_matrix, GLM_VEC3_ZERO);
    glm_mat4_identity(s_projection_matrix);
    glm_perspective(glm_rad(state.config.fov),
                    (float)state.config.width / (float)state.config.height, 0.1F,
                    (float)(state.config.render_distance + 1) * CHUNK_SIZE_XZ,
                    s_projection_matrix);

    // Cache uniform locations
    state.view_location = glGetUniformLocation(state.cube_shader.id, "view");
    state.projection_location = glGetUniformLocation(state.cube_shader.id, "projection");

    /* Replace the camera spawn height with player spawn */
    /* The camera init position will be overwritten by player_update,
       but we still need it for initial vector setup */
    camera_init(&state.config, &s_main_camera, (vec3) {0.0F, 80.0F, 0.0F});

    /* Initalizing the global player variables */
    player_init(&s_player, &state.config, &s_main_camera, (vec3) {0.0F, 80.0F, 0.0F});

    return state;
}

GLFWwindow* glfw_gl_init(const int width, const int height, const char* title) {
    if (!glfwInit()) {
        (void)fprintf(stderr, "GLFW could not initialize. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* game_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (game_window == NULL) {
        (void)fprintf(stderr, "%s\n", "Failed to create GLFW window.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(game_window);

    /* Setting all of our callbacks for various events such as moving
     * the cursor, clicking on the window, or resizing the window.*/
    glfwSetKeyCallback(game_window, key_callback);
    glfwSetCursorPosCallback(game_window, mouse_callback);
    glfwSetMouseButtonCallback(game_window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(game_window, framebuffer_size_callback);

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
    glfwGetFramebufferSize(game_window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    /* Set drawing mode (wireframe or full polygons) */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* Enabling depth-testing so that OpenGL uses its Z-buffer to
    prioritze drawings vertices that are closer to the camera. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return game_window;
}

static void game_loop(GLFWwindow* game_window, game_state_t* state) {
    static float wish_forward = 0.0F;
    static float wish_right = 0.0F;
    static float last_frame = 0.0F;
    static bool jump_pressed = false;
    static bool sprint = false;
    static vec3* pov_origin;
    static int last_render_distance = MAX_RENDER_DISTANCE;

    game_config_t* config = &state->config;

    /* Main window loop */
    while (!glfwWindowShouldClose(game_window)) {
        /* Calling this function allows us to gather all inputs
         * from the user such as mouse or keyboard. */
        glfwPollEvents();

        /* Background color */
        glClearColor(config->sky_color[0], config->sky_color[1], config->sky_color[2],
                     config->sky_color[3]);

        /* When clearing, we need to clear the buffer bit and also
         * the depth buffer bit so that information does not stack.
         * We can use a bitwise OR to do both in one call. Very
         * useful ! */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        handle_debug_inputs(game_window, config, &s_world);

        const float current_frame = (float)glfwGetTime();
        const float delta_time = current_frame - last_frame;

        last_frame = current_frame;
        pov_origin = config->free_camera ? &s_main_camera.position : &s_player.position;

        /* Apply the view and projection matrices */
        shader_use(&state->cube_shader);
        glUniformMatrix4fv(state->view_location, 1, GL_FALSE, (float*)s_view_matrix);
        glUniformMatrix4fv(state->projection_location, 1, GL_FALSE,
                           (float*)s_projection_matrix);
        shader_set_vec3(&state->cube_shader, "camera_position", (float*)*pov_origin);

        if (config->render_distance != last_render_distance) {
            glm_perspective(
                glm_rad(config->fov), ((float)config->width / (float)config->height), 0.1F,
                (float)(config->render_distance + 1) * CHUNK_SIZE_XZ, s_projection_matrix);
            world_reload(&s_world, config->render_distance);
            reload_fog(state);
            last_render_distance = config->render_distance;
        }

        /* Calculating the planes that make up the frustum of
         * the camera then culling everything that is outside the
         * frustum of the camera. Fortunately the CGLM libraries
         * has useful functions for this. */
        mat4 view_projection;
        glm_mat4_mul(s_projection_matrix, s_view_matrix, view_projection);
        vec4 frustum_planes[6];
        glm_frustum_planes(view_projection, frustum_planes);
        world_draw(&s_world, &state->cube_shader, &state->atlas, frustum_planes);

        /* Stream in/out chunks based on player position. */
        const int memcheck = world_update(&s_world, *pov_origin);
        if (memcheck < 0) {
            (void)fprintf(stderr,
                          "Memory allocation failure in world_update, exiting.\n");
            break;
        }

        camera_update_view(&s_main_camera, s_view_matrix);

        /* The freecam still updates the player's position so that the world
         * can keep loading. */
        if (config->free_camera) {
            handle_camera_mouse(game_window, config, &s_player, delta_time);
            vec3 player_updated_position = {s_player.camera->position[0],
                                            s_player.camera->position[1] -
                                                s_player.eye_offset,
                                            s_player.camera->position[2]};
            glm_vec3_copy(GLM_VEC3_ZERO, s_player.velocity);
            glm_vec3_copy(player_updated_position, s_player.position);
        } else {
            handle_player_input(game_window, &wish_forward, &wish_right, &jump_pressed,
                                &sprint);
            player_update(&s_player, config, &s_world, s_player.camera, wish_forward,
                          wish_right, jump_pressed, sprint, delta_time);

            const uint8_t block = get_pointed_block(
                &s_world, &s_main_camera, config->max_reach, &state->target_block,
                &state->neighbour_block, &state->target_chunk, &state->neighbour_chunk);
            if (block != (uint8_t)BLOCK_AIR) {
                if (handle_clicks(game_window, &s_world, &s_player, state->target_block,
                                  state->neighbour_block, state->target_chunk,
                                  state->neighbour_chunk)) {
                    (void)fprintf(stderr,
                                  "Chunk building failed after handle_click, exiting.\n");
                    break;
                };
            }
        }

        draw_debug_info(&state->debug_text_renderer, state);

        /* Swapping the buffers is a necessary step and I forgot
         * why. */
        glfwSwapBuffers(game_window);
    }
}

void draw_debug_info(const text_renderer_t* text_renderer,
                     const game_state_t* game_state) {
    /* Draw game title in the bottom-left corner */

    const player_t player = *game_state->player;
    const game_config_t config = game_state->config;

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

static void game_shutdown(GLFWwindow* window, const game_state_t* game_state) {
    shader_destroy(&game_state->cube_shader);
    text_renderer_destroy(&game_state->debug_text_renderer);
    material_destroy(&game_state->atlas);
    world_destroy(game_state->world);
    glfwDestroyWindow(window);
    glfwTerminate();
}