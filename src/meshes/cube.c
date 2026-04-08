#include "cglm/vec3.h"
#include <stdint.h>
#include <cglm/cglm.h>
#include <meshes/cube.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <shader.h>

// clang-format off

/**
 * @brief Listing all vertices needed to draw a cube */
static float CUBE_VERTICES[] = {
    // Back face
    -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
    // Front face
    -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
    // Left face
    -0.5f, -0.5f,  0.5f,  -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,
    // Right face
     0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,
    // Top face
    -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
    // Bottom face
    -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
};

/**
 * @brief Indices of the triangles needed to draw a cube */
static unsigned int CUBE_INDICES[] = {
     0,  1,  2,   2,  3,  0,  // back
     4,  5,  6,   6,  7,  4,  // front
     8,  9, 10,  10, 11,  8,  // left
    12, 13, 14,  14, 15, 12,  // right
    16, 17, 18,  18, 19, 16,  // top
    20, 21, 22,  22, 23, 20,  // bottom
};

static float CUBE_TEXTURE_COORDINATES[] = {
    0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,  // back
    0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,  // front
    0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,  // left
    0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,  // right
    0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,  // top
    0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,  // bottom
};

// clang-format on

const static unsigned int CUBE_VERTEX_COUNT = 24;
const static unsigned int CUBE_INDEX_COUNT = 36;

static mesh_t base_cube_mesh = {.vertices = CUBE_VERTICES,
                                .indices = CUBE_INDICES,
                                .texture_coordinates =
                                    CUBE_TEXTURE_COORDINATES,
                                .vertex_count = CUBE_VERTEX_COUNT,
                                .index_count = CUBE_INDEX_COUNT,
                                .vao = 0,
                                .vbos = {0},
                                .eao = 0
};

static size_t cube_count = 0;

void cube_init(cube_t* c) {
	++cube_count;
        c->mesh = &base_cube_mesh;

	glm_mat4_identity(c->model);
        glm_vec3_zero(c->position);
        glm_vec3_one(c->scale);
        glm_vec3_zero(c->rotation);

        if (c->mesh->vao == 0) glGenVertexArrays(1, &c->mesh->vao);
        glBindVertexArray(c->mesh->vao);

        if (c->mesh->vbos[0] == 0 || c->mesh->vbos[1] == 0)
                glGenBuffers(2, c->mesh->vbos);

        /* Setting position coordinates in first VBO */
        glBindBuffer(GL_ARRAY_BUFFER, c->mesh->vbos[0]);
        glBufferData(GL_ARRAY_BUFFER,
                     c->mesh->vertex_count * 3 * sizeof(float),
                     c->mesh->vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        /* Setting texture coordinates in second VBO */
        glBindBuffer(GL_ARRAY_BUFFER, c->mesh->vbos[1]);
        glBufferData(GL_ARRAY_BUFFER,
                     c->mesh->vertex_count * 2 * sizeof(float),
                     c->mesh->texture_coordinates, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(1);

        /* Setting up element drawing */
        if (c->mesh->eao == 0) glGenBuffers(1, &c->mesh->eao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->mesh->eao);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     c->mesh->index_count * sizeof(unsigned int),
                     c->mesh->indices, GL_STATIC_DRAW);
}

void cube_update(cube_t *c) {
	glm_mat4_identity(c->model);
	glm_translate(c->model, (float*) c->position);
	glm_rotate(c->model, c->rotation[0], (vec3) {1.0f, 0.0f, 0.0f});
	glm_rotate(c->model, c->rotation[1], (vec3) {0.0f, 1.0f, 0.0f});
	glm_rotate(c->model, c->rotation[2], (vec3) {0.0f, 0.0f, 0.0f});
	glm_scale(c->model, (float*) c->scale);
}

void cube_draw(cube_t* c, shader_t* s) {		
	shader_set_mat4(s, "model", c->model);	
        glBindVertexArray(c->mesh->vao);
        glDrawElements(GL_TRIANGLES, c->mesh->index_count, GL_UNSIGNED_INT,
                       0);
        glBindVertexArray(0);
}

void cube_free(cube_t* c) {
	--cube_count;
	if (cube_count) return;
	glDeleteVertexArrays(1, &c->mesh->vao);
	glDeleteBuffers(2, c->mesh->vbos);
	glDeleteBuffers(1, &c->mesh->eao);
}
