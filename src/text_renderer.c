#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <glad/gl.h>
#include <cglm/cglm.h>

#include "text_renderer.h"
#include "utils/io_utils.h"

/* Ensure our opaque buffer matches the real struct size. */
typedef char stbtt_size_check[(sizeof(stbtt_bakedchar) == TEXT_RENDERER_BAKED_CHAR_SIZE) ? 1 : -1];

void text_renderer_init(text_renderer_t* renderer, const char* font_path, const char* vert_path,
                        const char* frag_path, const float font_size, const int screen_width,
                        const int screen_height) {
    /* Load the TTF file into memory. */
    long font_size_bytes = 0;
    const unsigned char* font_data = NULL;
    read_file_bytes(font_path, &font_data, &font_size_bytes);

    if (!font_data) {
        (void)fprintf(stderr, "text_renderer: failed to load font\n");
        return;
    }

    /* Bake ASCII printable characters into a grayscale bitmap. */
    unsigned char bitmap[TEXT_RENDERER_BITMAP_SIZE * TEXT_RENDERER_BITMAP_SIZE];
    stbtt_bakedchar* const cdata = (stbtt_bakedchar*)renderer->cdata;
    const int rows_used = stbtt_BakeFontBitmap(
        font_data, 0, font_size, bitmap, TEXT_RENDERER_BITMAP_SIZE, TEXT_RENDERER_BITMAP_SIZE,
        TEXT_RENDERER_FIRST_CHAR, TEXT_RENDERER_NUM_CHARS, cdata);

    free((void*)font_data);

    if (rows_used < 0) {
        (void)fprintf(stderr, "text_renderer: font bitmap too small, %d chars didn't fit\n",
                      -rows_used);
    }

    /* Upload the bitmap as a single-channel texture. */
    glGenTextures(1, &renderer->texture_id);
    glBindTexture(GL_TEXTURE_2D, renderer->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXT_RENDERER_BITMAP_SIZE, TEXT_RENDERER_BITMAP_SIZE, 0,
                 GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Compile the text shaders. */
    shader_init(&renderer->shader, vert_path, frag_path);

    /* Set the orthographic projection (y-down: origin at top-left). */
    mat4 projection_matrix;
    glm_ortho(0.0F, (float)screen_width, (float)screen_height, 0.0F, -1.0F, 1.0F,
              projection_matrix);
    shader_set_mat4(&renderer->shader, "projection", projection_matrix);
    shader_set_int(&renderer->shader, "text_texture", 0);

    /* Set up a dynamic VAO/VBO for quad batching. */
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, (long)TEXT_RENDERER_MAX_CHARS * 6 * 4 * (GLsizeiptr)sizeof(float),
                 NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void text_renderer_draw(const text_renderer_t* renderer, const char* text, const float text_x,
                        const float text_y, const float text_color_r, const float text_color_g,
                        const float text_color_b) {
    /* Build a batch of quads for all printable characters. */
    float vertices[TEXT_RENDERER_MAX_CHARS * 6 * 4];
    int quad_count = 0;
    float coord_x = text_x;
    float coord_y = text_y;
    const stbtt_bakedchar* const cdata = (const stbtt_bakedchar*)renderer->cdata;

	/* For each character in text that we want to draw, we will create a quad with two triangles
	 * to store the single letter. */
    for (const char* ch = text; *ch != '\0' && quad_count < TEXT_RENDERER_MAX_CHARS; ch++) {
        const unsigned char letter = (unsigned char)*ch;
        if (letter >= TEXT_RENDERER_FIRST_CHAR &&
            letter < TEXT_RENDERER_FIRST_CHAR + TEXT_RENDERER_NUM_CHARS) {
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(cdata, TEXT_RENDERER_BITMAP_SIZE, TEXT_RENDERER_BITMAP_SIZE,
                               (int)(letter - TEXT_RENDERER_FIRST_CHAR), &coord_x, &coord_y, &quad,
                               1);

            float* vertex = &vertices[(ptrdiff_t)(quad_count * 24)];
            /* Triangle 1 */
            vertex[0] = quad.x0;
            vertex[1] = quad.y0;
            vertex[2] = quad.s0;
            vertex[3] = quad.t0;
            vertex[4] = quad.x1;
            vertex[5] = quad.y0;
            vertex[6] = quad.s1;
            vertex[7] = quad.t0;
            vertex[8] = quad.x1;
            vertex[9] = quad.y1;
            vertex[10] = quad.s1;
            vertex[11] = quad.t1;
            /* Triangle 2 */
            vertex[12] = quad.x0;
            vertex[13] = quad.y0;
            vertex[14] = quad.s0;
            vertex[15] = quad.t0;
            vertex[16] = quad.x1;
            vertex[17] = quad.y1;
            vertex[18] = quad.s1;
            vertex[19] = quad.t1;
            vertex[20] = quad.x0;
            vertex[21] = quad.y1;
            vertex[22] = quad.s0;
            vertex[23] = quad.t1;
            quad_count++;
        }
    }

    if (quad_count == 0) { return; }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    shader_use(&renderer->shader);
    glUniform3f(glGetUniformLocation(renderer->shader.id, "text_color"), text_color_r, text_color_g,
                text_color_b);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->texture_id);

    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)((size_t)(quad_count * 24) * sizeof(float)),
                    vertices);
    glDrawArrays(GL_TRIANGLES, 0, quad_count * 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void text_renderer_destroy(const text_renderer_t* renderer) {
    glDeleteTextures(1, &renderer->texture_id);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
    shader_destroy(&renderer->shader);
}
