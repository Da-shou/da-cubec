#include "main.h"

#include <cglm/cglm.h>

#include "material.h"
#include "callbacks.h"
#include "input_process.h"
#include "hud/debug_ui.h"
#include "world/pointer.h"
#include "world/fog.h"
#include "world/blocks.h"
#include "world/world.h"

/**
 * @brief Main function of the game. Initializes the game window using GLFW, then creates
 * a base state for the game. Finally, launches the game render loop with the created
 * state. Shutdowns when the loop is exited.
 * @return void
 */
int main(void) {
    GLFWwindow* game_window =
        glfw_gl_init(INITIAL_WIDTH, INITIAL_HEIGHT, INITIAL_WINDOW_TITLE);
    game_state_t state = game_state_init();
    glfwSetWindowUserPointer(game_window, &state);
    game_loop(game_window, &state);
    game_shutdown(game_window, &state);
    return EXIT_SUCCESS;
}

game_state_t game_state_init(void) {
    /* Zero initialization of our state object */
    game_state_t state = {0};

    state.config = game_config_default();
    state.world  = world;
    state.player = &s_player;

    /* Shader and material creation */
    shader_init(&state.cube_shader, state.config.cube_vertex_shader_path,
                state.config.cube_fragment_shader_path);
    material_create(&state.atlas, state.config.texture_atlas_path);

    /* Debug text renderer initalization */
    text_renderer_init(
        &state.debug_text_renderer, state.config.debug_font_path,
        state.config.text_vertex_shader_path, state.config.text_fragment_shader_path,
        state.config.debug_font_size, state.config.width, state.config.height);

    /* World generation using perlin noise for the hills */
    world_init(&state.world, &state.config);
    static perlin_params_t terrain = {0.01F, 64, 32};
    state.world->generate          = world_generator_perlin;
    state.world->generator_data    = &terrain;

    /* View and projection matrices calculations, setting the FOV and the min/max drawing
     * distance. */
    glm_mat4_identity(s_view_matrix);
    glm_translate(s_view_matrix, GLM_VEC3_ZERO);
    glm_mat4_identity(s_projection_matrix);
    glm_perspective(glm_rad(state.config.fov),
                    (float)state.config.width / (float)state.config.height, 0.1F,
                    (float)(state.config.render_distance + 1) * CHUNK_SIZE_XZ,
                    s_projection_matrix);

    /* Caching uniform locations of the view and projections matrices in the cube shader.
     */
    state.view_location       = glGetUniformLocation(state.cube_shader.id, "view");
    state.projection_location = glGetUniformLocation(state.cube_shader.id, "projection");

    /* Player initalization */
    player_init(state.player, &state.config, &s_main_camera, (vec3) {0.0F, 80.0F, 0.0F});

    /* Camera initalization at same position as player */
    camera_init(&state.config, state.player->camera, (vec3) {0.0F, 80.0F, 0.0F});

    world_update(state.world, (vec3) {0.0F, 80.0F, 0.0F});
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
    printf("\nStarting GLFW\n");
    printf("Running on OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
           GLAD_VERSION_MINOR(version));
    printf("Compiled against GLFW %i.%i.%i\n", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR,
           GLFW_VERSION_REVISION);
    int major;
    int minor;
    int revision;
    glfwGetVersion(&major, &minor, &revision);
    printf("Running against GLFW %i.%i.%i\n", major, minor, revision);

    const char* platform;
    switch (glfwGetPlatform()) {
    case GLFW_PLATFORM_WIN32: platform = "Windows"; break;
    case GLFW_PLATFORM_WAYLAND: platform = "Linux (Wayland)"; break;
    case GLFW_PLATFORM_X11: platform = "Linux (X11)"; break;
    case GLFW_PLATFORM_COCOA: platform = "MacOS"; break;
    default: platform = "Unrecognized"; break;
    }
    printf("Platform is %s\n", platform);

    const GLubyte* gpu_vendor   = glGetString(GL_VENDOR);
    const GLubyte* gpu_renderer = glGetString(GL_RENDERER);
    const GLubyte* gpu_version  = glGetString(GL_VERSION);
    const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU Vendor is %s\n", (const char*)gpu_vendor);
    printf("GPU Renderer is %s\n", (const char*)gpu_renderer);
    printf("GPU OpenGL Version is %s\n", (const char*)gpu_version);
    printf("GLSL Version is %s\n", (const char*)glsl_version);

    int extensions_count;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensions_count);
    printf("Loaded %d OpenGL extensions\n", extensions_count);

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

void game_loop(GLFWwindow* game_window, game_state_t* state) {
    static float wish_forward = 0.0F;
    static float wish_right   = 0.0F;
    static float last_frame   = 0.0F;
    static bool jump_pressed  = false;
    static bool sprint        = false;
    static vec3* pov_origin;
    static int last_render_distance = MAX_RENDER_DISTANCE;

    const game_config_t* config = &state->config;

    /* Setting the requested width and height after loading the configuration */
    glfwSetWindowSize(game_window, state->config.width, state->config.height);

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

        /* Handling the different debug inputs such as freecam
         * toggle or reloading chunks */
        handle_debug_inputs(game_window, state);

        /* Calculating the delta_time */
        const float current_frame = (float)glfwGetTime();
        const float delta_time    = current_frame - last_frame;
        last_frame                = current_frame;

        /* Setting the correct origin from which to render the world, either the camera or
         * the player depending on the free_camera boolean in the config */
        camera_t* pcamera = state->player->camera;
        pov_origin = config->free_camera ? &pcamera->position : &state->player->position;

        /* Apply the view and projection matrices */
        shader_use(&state->cube_shader);
        glUniformMatrix4fv(state->view_location, 1, GL_FALSE, (float*)s_view_matrix);
        glUniformMatrix4fv(state->projection_location, 1, GL_FALSE,
                           (float*)s_projection_matrix);
        shader_set_vec3(&state->cube_shader, "camera_position", *pov_origin);

        /* If the render distance was changed by the user, the world needs to be reloaded,
         * as well as the projection matrix and the fog since they all depend on the
         * render distance value.*/
        if (config->render_distance != last_render_distance) {
            glm_perspective(glm_rad(config->fov),
                            (float)config->width / (float)config->height, 0.1F,
                            (float)(config->render_distance + 1) * CHUNK_SIZE_XZ,
                            s_projection_matrix);
            world_reload(state->world, config->render_distance);
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

        /* The freecam still updates the player's position so that the world
         * can keep loading. */
        if (config->free_camera) {
            handle_camera_mouse(game_window, config, state->player, delta_time);
            vec3 player_updated_position = {
                pcamera->position[0], pcamera->position[1] - state->player->eye_offset,
                pcamera->position[2]};
            glm_vec3_copy(GLM_VEC3_ZERO, state->player->velocity);
            glm_vec3_copy(player_updated_position, state->player->position);
        } else if (world_player_chunks_ready(state->world, state->player->position)) {
            /* Gets the input about the player movement and
             * updates the direction integers */
            handle_player_input(game_window, &wish_forward, &wish_right, &jump_pressed,
                                &sprint);

            /* Applies gravity, movement vectors, checks collisions */
            player_update(state->player, config, state->world, pcamera, wish_forward,
                          wish_right, jump_pressed, sprint, delta_time);

            /* Checks what block is currently being pointer at */
            const uint8_t block = get_pointed_block(state, state->config.max_reach);

            /* Only handling clicks if the block pointed to is not air. */
            if (block != (uint8_t)BLOCK_AIR && state->target_chunk != NULL) {
                if (handle_clicks(game_window, state->world, state->player,
                                  state->target_block, state->neighbour_block,
                                  state->target_chunk, state->neighbour_chunk)) {
                    (void)fprintf(stderr,
                                  "Chunk building failed after handle_click, exiting.\n");
                    break;
                };
            }
        } else {
            glm_vec3_zero(state->player->velocity);
        }

        /* Stream in/out chunks based on player position. */
        const int memcheck = world_update(state->world, *pov_origin);
        if (memcheck < 0) {
            (void)fprintf(stderr,
                          "Memory allocation failure in world_update, exiting.\n");
            break;
        }

        /* Creates the direction vector depending on the current front vector of the
         * camera and updates the view matrix according to it. */
        camera_update_view(pcamera, s_view_matrix);

        /* Drawing the world first, if not the UI will get drawn behind the world. */
        world_draw(state->world, &state->cube_shader, &state->atlas, frustum_planes);

        /* Draws text for coordinates, render distance, OpenGL and GLFW version, and game
         * version */
        draw_debug_info(&state->debug_text_renderer, state);

        /* Swapping the buffers is a necessary step and I forgot
         * why. */
        glfwSwapBuffers(game_window);
    }
}

void game_shutdown(GLFWwindow* window, const game_state_t* game_state) {
    shader_destroy(&game_state->cube_shader);
    text_renderer_destroy(&game_state->debug_text_renderer);
    material_destroy(&game_state->atlas);
    world_destroy(game_state->world);
    glfwDestroyWindow(window);
    glfwTerminate();
}
