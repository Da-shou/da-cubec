/**
 * @file text_renderer.h
 * @authors Da-shou
 * @brief Header file declaring a struct containing all useful elements to create a
 * text_renderer, with a certain font and size.
 */

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "opengl_glfw.h"
#include "shader.h"

#define TEXT_RENDERER_FIRST_CHAR 32
#define TEXT_RENDERER_NUM_CHARS 96
#define TEXT_RENDERER_BITMAP_SIZE 512
#define TEXT_RENDERER_MAX_CHARS 256

/* sizeof(stbtt_bakedchar) */
#define TEXT_RENDERER_BAKED_CHAR_SIZE 20

/** Stores all attributes necessary for drawing text on the screen. */
typedef struct {
    shader_t shader;
    GLuint texture_id;
    GLuint vao;
    GLuint vbo;
    /* Opaque storage for stbtt_bakedchar[TEXT_RENDERER_NUM_CHARS]. */
    unsigned char cdata[TEXT_RENDERER_NUM_CHARS * TEXT_RENDERER_BAKED_CHAR_SIZE];
    float font_size;
} text_renderer_t;

/**
 * @brief Initializes a text renderer by loading a TTF font and baking it into
 *        a GPU texture.
 * @param renderer Text renderer to initialize.
 * @param font_path Path to a TTF font file.
 * @param vert_path Path to the text vertex shader.
 * @param frag_path Path to the text fragment shader.
 * @param font_size Font size in pixels.
 * @param screen_width Width of the render target in pixels.
 * @param screen_height Height of the render target in pixels. */
void text_renderer_init(text_renderer_t* renderer, const char* font_path,
                        const char* vert_path, const char* frag_path, float font_size,
                        int screen_width, int screen_height);

/**
 * @brief Draws a string at the given screen position.
 * @param renderer Text renderer to use.
 * @param text Null-terminated string to draw.
 * @param text_pos Position of the text in XY coordinates.
 * @param text_color color of the text in RGB format.
 * @param draw_background if the text should have a background
 * @param bg_color color of the background in RGB format. NULL if no background.
 * @param bg_alpha tranparancy of the background. */
void text_renderer_draw(const text_renderer_t* renderer, const char* text,
                        const vec2 text_pos, const vec3 text_color, bool draw_background,
                        const vec3 bg_color, float bg_alpha);

/**
 * @brief Frees GPU resources held by a text renderer.
 * @param renderer Text renderer to destroy. */
void text_renderer_destroy(const text_renderer_t* renderer);

#endif // TEXT_RENDERER_H
