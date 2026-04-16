#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <cglm/cglm.h>
#include <stdint.h>
#include <string.h>

#include "world/chunk.h"
#include "world/blocks.h"
#include "shader.h"
#include "material.h"

/* These are all the different vertices for a face that are needed. */
// clang-format off
static bool face_front[4][3] = {
    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};

static bool face_back[4][3] = {
    {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}};

static bool face_left[4][3] = {
    {0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}};

static bool face_right[4][3] = {
    {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}};

static bool face_top[4][3] = {
    {0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}};

static bool face_bottom[4][3] = {
    {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}};
// clang-format on

void chunk_init(chunk_t* chunk, vec3 position) {
    glm_vec3_copy(position, chunk->position);
    memset(chunk->blocks, 0, sizeof(chunk->blocks));
    chunk_mesh_init(&chunk->mesh);
}

void chunk_mesh_init(chunk_mesh_t* mesh) {
    mesh->vertex_capacity = starting_chunk_capacity;
    mesh->index_capacity = starting_chunk_capacity;
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    mesh->vertices =
        (chunk_vertex_t*)malloc(mesh->vertex_capacity * sizeof(chunk_vertex_t));
    mesh->indices = (unsigned int*)malloc(mesh->vertex_capacity * sizeof(unsigned int));
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->eao);
}

int chunk_mesh_push_face(chunk_mesh_t* mesh, const uint8_t face_x, const uint16_t face_y,
                         const uint8_t face_z, bool face_vertices[4][3],
                         const float uv_offset_x, const float uv_offset_y,
                         const float uv_size) {
    /* Checking if vertices and indices array need to be reallocated.
     * To add a face, we need 4 new vertices. And for each face added,
     * we need 6 indices each. */
    if (mesh->vertex_count + 4 > mesh->vertex_capacity) {
        mesh->vertex_capacity *= 2;
        void* new_space =
            realloc(mesh->vertices, mesh->vertex_capacity * sizeof(chunk_vertex_t));
        if (new_space == NULL) {
            free(mesh->vertices);
            return -1;
        }
        mesh->vertices = new_space;
    }

    if (mesh->index_count + 6 > mesh->index_capacity) {
        mesh->index_capacity *= 2;
        void* new_space =
            realloc(mesh->indices, mesh->index_capacity * sizeof(unsigned int));
        if (new_space == NULL) {
            free(mesh->indices);
            return -1;
        }
        mesh->indices = new_space;
    }

    /* Storing the UV corners for this atlas tile */
    const float uvs[4][2] = {
        {uv_offset_x, uv_offset_y + uv_size},
        {uv_offset_x + uv_size, uv_offset_y + uv_size},
        {uv_offset_x + uv_size, uv_offset_y},
        {uv_offset_x, uv_offset_y},
    };

    const size_t base = mesh->vertex_count;

    /* Adding the 4 vertices needed to draw a face to the mesh.
     * The values added to x, y and z are given from static arrays
     * allowing a quick access to the necessary offset for each
     * vertex.*/
    for (uint8_t i = 0; i < 4; ++i) {
        mesh->vertices[mesh->vertex_count++] =
            chunk_vertex_pack(face_x + face_vertices[i][0], face_y + face_vertices[i][1],
                              face_z + face_vertices[i][2], uvs[i][0], uvs[i][1]);
    }

    /* Here we push 6 indices, which will draw two triangles, which in
     * turn will draw a single face of the chunk. */
    mesh->indices[mesh->index_count++] = (unsigned int)(base + 0);
    mesh->indices[mesh->index_count++] = (unsigned int)(base + 1);
    mesh->indices[mesh->index_count++] = (unsigned int)(base + 2);
    mesh->indices[mesh->index_count++] = (unsigned int)(base + 2);
    mesh->indices[mesh->index_count++] = (unsigned int)(base + 3);
    mesh->indices[mesh->index_count++] = (unsigned int)(base + 0);

    return 0;
}

int chunk_build_mesh(const chunk_t* chunk, chunk_mesh_t* mesh,
                     const chunk_neighbours_t neighbors) {
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    /* Big ass check on ALL cubes and sending each facing that face
     * BLOCK_AIR to the chunk mesh to be built. */
    for (uint8_t block_x = 0; block_x < CHUNK_SIZE_XZ; ++block_x) {
        for (uint16_t block_y = 0; block_y < CHUNK_SIZE_Y; ++block_y) {
            for (uint8_t block_z = 0; block_z < CHUNK_SIZE_XZ; ++block_z) {
                const block_type_t block = chunk->blocks[block_x][block_y][block_z];
                if (block == BLOCK_AIR) { continue; }

                const block_uv_t uv_block = block_uvs[block];

                /* To determine if the face of a block in a chunk will be rendered, we
                 * check the 4 potentials neighbors (front, back, left and right). The top
                 * and bottom chunks d not exist. If a block is found in the neighbouring
                 * chunk, then the face is not rendered. If the chunk is at the edge of
                 * the world, then the face is rendered. */
                if (block_z == CHUNK_SIZE_XZ - 1) {
                    // Front-checking
                    if (!neighbors.north ||
                        neighbors.north->blocks[block_x][block_y][0] ==
                            (uint8_t)BLOCK_AIR) {
                        if (chunk_mesh_push_face(mesh, block_x, block_y, block_z,
                                                 face_front, uv_block.front.u,
                                                 uv_block.front.v, tile_offset)) {
                            return -1;
                        };
                    }
                } else if (chunk->blocks[block_x][block_y][block_z + 1] ==
                           (uint8_t)BLOCK_AIR) {
                    if (chunk_mesh_push_face(mesh, block_x, block_y, block_z, face_front,
                                             uv_block.front.u, uv_block.front.v,
                                             tile_offset)) {
                        return -1;
                    };
                }

                if (block_z == 0) {
                    // Back-checking
                    if (!neighbors.south ||
                        neighbors.south->blocks[block_x][block_y][CHUNK_SIZE_XZ - 1] ==
                            (uint8_t)BLOCK_AIR) {
                        if (chunk_mesh_push_face(mesh, block_x, block_y, block_z,
                                                 face_back, uv_block.back.u,
                                                 uv_block.back.v, tile_offset)) {
                            return -1;
                        };
                    }
                } else if (chunk->blocks[block_x][block_y][block_z - 1] ==
                           (uint8_t)BLOCK_AIR) {
                    if (chunk_mesh_push_face(mesh, block_x, block_y, block_z, face_back,
                                             uv_block.back.u, uv_block.back.v,
                                             tile_offset)) {
                        return -1;
                    };
                }

                if (block_y == CHUNK_SIZE_Y - 1 ||
                    // Above
                    chunk->blocks[block_x][block_y + 1][block_z] == (uint8_t)BLOCK_AIR) {
                    if (chunk_mesh_push_face(mesh, block_x, block_y, block_z, face_top,
                                             uv_block.top.u, uv_block.top.v,
                                             tile_offset)) {
                        return -1;
                    };
                }

                if (block_y == 0 ||
                    // Below
                    chunk->blocks[block_x][block_y - 1][block_z] == (uint8_t)BLOCK_AIR) {
                    if (chunk_mesh_push_face(mesh, block_x, block_y, block_z, face_bottom,
                                             uv_block.bottom.u, uv_block.bottom.v,
                                             tile_offset)) {
                        return -1;
                    };
                }

                if (block_x == CHUNK_SIZE_XZ - 1) {
                    // Right-checking
                    if (!neighbors.east || neighbors.east->blocks[0][block_y][block_z] ==
                                               (uint8_t)BLOCK_AIR) {
                        if (chunk_mesh_push_face(mesh, block_x, block_y, block_z,
                                                 face_right, uv_block.right.u,
                                                 uv_block.right.v, tile_offset)) {
                            return -1;
                        };
                    }
                } else if (chunk->blocks[block_x + 1][block_y][block_z] ==
                           (uint8_t)BLOCK_AIR) {
                    if (chunk_mesh_push_face(mesh, block_x, block_y, block_z, face_right,
                                             uv_block.right.u, uv_block.right.v,
                                             tile_offset)) {
                        return -1;
                    };
                }

                if (block_x == 0) {
                    // Left-checking
                    if (!neighbors.west ||
                        neighbors.west->blocks[CHUNK_SIZE_XZ - 1][block_y][block_z] ==
                            (uint8_t)BLOCK_AIR) {
                        if (chunk_mesh_push_face(mesh, block_x, block_y, block_z,
                                                 face_left, uv_block.left.u,
                                                 uv_block.left.v, tile_offset)) {
                            return -1;
                        };
                    }
                } else if (chunk->blocks[block_x - 1][block_y][block_z] ==
                           (uint8_t)BLOCK_AIR) {
                    if (chunk_mesh_push_face(mesh, block_x, block_y, block_z, face_left,
                                             uv_block.left.u, uv_block.left.v,
                                             tile_offset)) {
                        return -1;
                    };
                }
            }
        }
    }
    chunk_mesh_upload(mesh);
    return 0;
}

uint32_t chunk_vertex_pack(const uint8_t vertex_x, const uint16_t vertex_y,
                           const uint8_t vertex_z, const float uv_u, const float uv_v) {
    const uint32_t u_idx = (uint32_t)roundf(uv_u / tile_offset);
    const uint32_t v_idx = (uint32_t)roundf(uv_v / tile_offset);
    return (vertex_x & 0x1FU) | ((vertex_z & 0x1FU) << 5U) |
           ((vertex_y & 0x3FFU) << 10U) | (u_idx << 20U) | (v_idx << 23U);
}

void chunk_mesh_upload(const chunk_mesh_t* mesh) {
    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(mesh->vertex_count * sizeof(chunk_vertex_t)),
                 mesh->vertices, GL_DYNAMIC_DRAW);

    /** Since all of our data is packed into an uint32_t, no need to to
     * pass multiple attributes. The unpacking will be done in the
     * vertex shader. */
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(uint32_t), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eao);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(mesh->index_count * sizeof(unsigned int)), mesh->indices,
                 GL_DYNAMIC_DRAW);
}

void chunk_mesh_draw(const chunk_mesh_t* mesh) {
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, (int)mesh->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void chunk_draw(chunk_t* chunk, const shader_t* shader, const material_t* atlas) {
    glActiveTexture(GL_TEXTURE0);
    material_use(atlas, 0);
    shader_use(shader);
    shader_set_int(shader, "current_texture", 0);
    mat4 model;
    glm_mat4_copy(GLM_MAT4_IDENTITY, model);
    glm_translate(model, chunk->position);
    shader_set_mat4(shader, "model", model);
    chunk_mesh_draw(&chunk->mesh);
}

void chunk_mesh_destroy(chunk_mesh_t* mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    mesh->vertices = NULL;
    mesh->indices = NULL;
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    mesh->vertex_capacity = 0;
    mesh->index_capacity = 0;
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->eao);
    mesh->vao = 0;
    mesh->vbo = 0;
    mesh->eao = 0;
}

void chunk_destroy(chunk_t* chunk) {
    chunk_mesh_destroy(&chunk->mesh);
}
