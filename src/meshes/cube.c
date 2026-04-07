#include <stdint.h>
#include <cglm/cglm.h>
#include <meshes/cube.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

// clang-format off

/**
 * @brief Listing all vertices needed to draw a cube */
static float CUBE_VERTICES[] = {
    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f,  0.5f,  
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f,  
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f,  0.5f,  
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f, 
    -0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f,  
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f,  0.5f,  
    -0.5f, -0.5f,  0.5f,
};

/**
 * @brief Indices of the triangles needed to draw a cube */
static unsigned int CUBE_INDICES[] = {
    0,  1,  2,  
    2,  3,  0,  
    4,  5,  6,  
    6,  7,  4,  
    8,  9,  10, 
    10, 11, 8,
    12, 13, 14, 
    14, 15, 12, 
    16, 17, 18, 
    18, 19, 16, 
    20, 21, 22, 
    22, 23, 20,
};

static float CUBE_TEXTURE_COORDINATES[] = {
    0.0f, 0.0f, 
    1.0f, 0.0f, 
    1.0f, 1.0f, 
    0.0f, 1.0f, 
    0.0f, 0.0f, 
    1.0f, 0.0f,
    1.0f, 1.0f, 
    0.0f, 1.0f, 
    0.0f, 0.0f, 
    1.0f, 0.0f, 
    1.0f, 1.0f, 
    0.0f, 1.0f,
    0.0f, 0.0f, 
    1.0f, 0.0f, 
    1.0f, 1.0f, 
    0.0f, 1.0f, 
    0.0f, 0.0f, 
    1.0f, 0.0f,
    1.0f, 1.0f, 
    0.0f, 1.0f,
    0.0f, 0.0f, 
    1.0f, 0.0f, 
    1.0f, 1.0f, 
    0.0f, 1.0f,
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
                                .eao = 0};

void cube_init(cube_t* c) {
        c->mesh = &base_cube_mesh;

        glm_vec3_zero(c->position);
        glm_vec3_zero(c->scale);
        glm_vec3_zero(c->rotation);

        if (c->mesh->vao == 0) glGenVertexArrays(1, &c->mesh->vao);
        glBindVertexArray(c->mesh->vao);

        glGenBuffers(2, c->mesh->vbos);
        /* Setting position coordinates in first VBO */
        glBindBuffer(GL_ARRAY_BUFFER, c->mesh->vbos[0]);
        glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(float),
                     c->mesh->vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        /* Setting texture coordinates in second VBO */
        glBindBuffer(GL_ARRAY_BUFFER, c->mesh->vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(float),
                     c->mesh->texture_coordinates, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(1);

        /* Setting up element drawing */
        glGenBuffers(1, &c->mesh->eao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->mesh->eao);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     c->mesh->index_count * sizeof(uint64_t),
                     c->mesh->indices, GL_STATIC_DRAW);
}

void cube_draw(cube_t* c) {
        glBindVertexArray(c->mesh->vao);
        glDrawElements(GL_TRIANGLES, c->mesh->index_count, GL_UNSIGNED_INT,
                       0);
        glBindVertexArray(0);
}

void cube_free(cube_t* c) {
        glDeleteVertexArrays(1, &c->mesh->vao);
        glDeleteBuffers(2, c->mesh->vbos);
        glDeleteBuffers(1, &c->mesh->eao);
}
