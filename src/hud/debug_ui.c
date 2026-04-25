#include "hud/debug_ui.h"

#include <cglm/cglm.h>
#include <stdio.h>

#include "world/blocks.h"

#define DEBUG_LINES 5

void draw_debug_info(const text_renderer_t* text_renderer,
                     const game_state_t* game_state) {
    /* Draw game title in the bottom-left corner */

    const player_t player      = *game_state->player;
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
	const char* block_name = NULL;
	get_block_name(player.block, &block_name);
	char block_name_text[64];
	(void)snprintf(block_name_text, sizeof(block_name_text), "Selected block : %s", block_name);
	
    char* strings[DEBUG_LINES] = {render_distance_text, x_coordinates, y_coordinates,
                            z_coordinates, block_name_text};

    const float line_spacing  = text_renderer->font_size * 1.25F;
    const float margin_bottom = line_spacing - 20.0F;
    text_renderer_draw(text_renderer, title_text,
                       (vec2) {10.0F, (float)config.height - margin_bottom}, GLM_VEC3_ONE,
                       true, GLM_VEC3_ZERO, 0.33F);

    text_renderer_draw(
        text_renderer, opengl_info,
        (vec2) {10.0F, (float)config.height - margin_bottom - line_spacing}, GLM_VEC3_ONE,
        true, GLM_VEC3_ZERO, 0.33F);

    for (int8_t i = 0; i < DEBUG_LINES; i++) {
        const float margin_top = 5.0F;
        text_renderer_draw(text_renderer, strings[i],
                           (vec2) {10.0F, text_renderer->font_size + margin_top +
                                              (line_spacing * (float)i)},
                           GLM_VEC3_ONE, true, GLM_VEC3_ZERO, 0.33F);
    }
}
