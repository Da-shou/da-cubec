#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "shader.h"

#define TEXT_RENDERER_FIRST_CHAR 32
#define TEXT_RENDERER_NUM_CHARS 96
#define TEXT_RENDERER_BITMAP_SIZE 512
#define TEXT_RENDERER_MAX_CHARS 256

/* sizeof(stbtt_bakedchar) == 20 — verified in text_renderer.c */
#define TEXT_RENDERER_BAKED_CHAR_SIZE 20

/** Stores all attributes necessary for drawing text on the screen. */
typedef struct {
    shader_t shader;
    unsigned int texture_id;
    unsigned int vao;
    unsigned int vbo;
    /* Opaque storage for stbtt_bakedchar[TEXT_RENDERER_NUM_CHARS]. */
    unsigned char cdata[TEXT_RENDERER_NUM_CHARS * TEXT_RENDERER_BAKED_CHAR_SIZE];
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
 * @param text_x Horizontal position of the text baseline start (pixels from left).
 * @param text_y Vertical position of the text baseline (pixels from top).
 * @param text_color_r Red component of the text color (0–1).
 * @param text_color_g Green component of the text color (0–1).
 * @param text_color_b Blue component of the text color (0–1). */
void text_renderer_draw(const text_renderer_t* renderer, const char* text, float text_x,
                        float text_y, float text_color_r, float text_color_g,
                        float text_color_b);

/**
 * @brief Frees GPU resources held by a text renderer.
 * @param renderer Text renderer to destroy. */
void text_renderer_destroy(const text_renderer_t* renderer);

#endif // TEXT_RENDERER_H
