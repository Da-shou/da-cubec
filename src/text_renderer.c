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
typedef char
    stbtt_size_check[(sizeof(stbtt_bakedchar) == TEXT_RENDERER_BAKED_CHAR_SIZE) ? 1 : -1];

void text_renderer_init(text_renderer_t* renderer, const char* font_path,
                        const char* vert_path, const char* frag_path,
                        const float font_size, const int screen_width,
                        const int screen_height) {
    /* Load the TTF file into memory. */
    long font_size_bytes           = 0;
    const unsigned char* font_data = NULL;
    read_file_bytes(font_path, &font_data, &font_size_bytes);

    if (!font_data) {
        (void)fprintf(stderr, "text_renderer: failed to load font\n");
        return;
    }

    /* Bake ASCII printable characters into a grayscale bitmap. */
    unsigned char bitmap[TEXT_RENDERER_BITMAP_SIZE * TEXT_RENDERER_BITMAP_SIZE];
    stbtt_bakedchar* const cdata = (stbtt_bakedchar*)renderer->cdata;
    const int rows_used =
        stbtt_BakeFontBitmap(font_data, 0, font_size, bitmap, TEXT_RENDERER_BITMAP_SIZE,
                             TEXT_RENDERER_BITMAP_SIZE, TEXT_RENDERER_FIRST_CHAR,
                             TEXT_RENDERER_NUM_CHARS, cdata);

    free((void*)font_data);
    renderer->font_size = font_size;

    if (rows_used < 0) {
        (void)fprintf(stderr,
                      "text_renderer: font bitmap too small, %d chars didn't fit\n",
                      -rows_used);
    }

    /* Upload the bitmap as a single-channel texture. */
    glGenTextures(1, &renderer->texture_id);
    glBindTexture(GL_TEXTURE_2D, renderer->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXT_RENDERER_BITMAP_SIZE,
                 TEXT_RENDERER_BITMAP_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
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
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)TEXT_RENDERER_MAX_CHARS * 6 * 4 * (GLsizeiptr)sizeof(float),
                 NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void text_renderer_draw(const text_renderer_t* renderer, const char* text,
                        const vec2 text_pos, const vec3 text_color,
                        const bool draw_background, const vec3 bg_color,
                        const float bg_alpha) {
    if (draw_background == false) { bg_color = NULL; }

    /* Build a batch of quads for all printable characters. */
    float vertices[TEXT_RENDERER_MAX_CHARS * 6 * 4];
    int quad_count                     = 0;
    float coord_x                      = text_pos[0];
    float coord_y                      = text_pos[1];
    const stbtt_bakedchar* const cdata = (const stbtt_bakedchar*)renderer->cdata;

    /* Track bounding box for the background. */
    float min_x = coord_x;
    float max_x = coord_x;
    float min_y = coord_y;
    float max_y = coord_y;

    /* For each character in text that we want to draw, we will create a quad with two
     * triangles to store the single letter. */
    for (const char* ch = text; *ch != '\0' && quad_count < TEXT_RENDERER_MAX_CHARS;
         ch++) {
        const unsigned char letter = (unsigned char)*ch;
        if (letter >= TEXT_RENDERER_FIRST_CHAR &&
            letter < TEXT_RENDERER_FIRST_CHAR + TEXT_RENDERER_NUM_CHARS) {
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(
                cdata, TEXT_RENDERER_BITMAP_SIZE, TEXT_RENDERER_BITMAP_SIZE,
                (letter - TEXT_RENDERER_FIRST_CHAR), &coord_x, &coord_y, &quad, 1);

            /* Expand bounding box. */
            if (quad.x0 < min_x) { min_x = quad.x0; }
            if (quad.x1 > max_x) { max_x = quad.x1; }
            if (quad.y0 < min_y) { min_y = quad.y0; }
            if (quad.y1 > max_y) { max_y = quad.y1; }

            float* vertex = &vertices[(ptrdiff_t)(quad_count * 24)];
            /* Triangle 1 */
            vertex[0]  = quad.x0;
            vertex[1]  = quad.y0;
            vertex[2]  = quad.s0;
            vertex[3]  = quad.t0;
            vertex[4]  = quad.x1;
            vertex[5]  = quad.y0;
            vertex[6]  = quad.s1;
            vertex[7]  = quad.t0;
            vertex[8]  = quad.x1;
            vertex[9]  = quad.y1;
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

    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)((size_t)(quad_count * 24) * sizeof(float)), vertices);

    if (draw_background) {
        const float padding        = 4.0F;
        const float background[24] = {
            min_x - padding, min_y - padding, 0.0F, 0.0F,
            max_x + padding, min_y - padding, 0.0F, 0.0F,
            max_x + padding, max_y + padding, 0.0F, 0.0F,
            max_x + padding, max_y + padding, 0.0F, 0.0F,
            min_x - padding, max_y + padding, 0.0F, 0.0F,
            min_x - padding, min_y - padding, 0.0F, 0.0F,
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(background), background);
        shader_use(&renderer->shader);
        /* Temporarily pass bg color as text_color, alpha via a new uniform. */
        shader_set_vec3(&renderer->shader, "text_color", (float*)bg_color);
        shader_set_float(&renderer->shader, "force_alpha", bg_alpha);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        /* Re-upload the real text vertices. */
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        (GLsizeiptr)((size_t)(quad_count * 24) * sizeof(float)),
                        vertices);
    }

    shader_use(&renderer->shader);
    shader_set_vec3(&renderer->shader, "text_color", (float*)text_color);
    shader_set_float(&renderer->shader, "force_alpha", -1.0F); // -1 = sample from texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->texture_id);
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
