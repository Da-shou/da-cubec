#include "world/chunk.h"

#include <cglm/cglm.h>
#include <stdint.h>
#include <string.h>

#include "world/blocks.h"
#include "shader.h"
#include "material.h"

#define LIGHT_TOP 1.0F
#define LIGHT_FRONT 0.8F
#define LIGHT_BACK 0.6F
#define LIGHT_LEFT 0.6F
#define LIGHT_RIGHT 0.6F
#define LIGHT_BOTTOM 0.4F
#define MAX_LIGHT 15
#define DAY_LIGHT 7
#define LIGHT_FALLOFF 1

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
    chunk->needs_rebuild       = false;
    chunk->needs_light_rebuild = false;
    chunk->modified            = false;
    chunk->ready               = false;
}

void chunk_mesh_init(chunk_mesh_t* mesh) {
    mesh->vertex_capacity = starting_chunk_capacity;
    mesh->index_capacity  = starting_chunk_capacity;
    mesh->vertex_count    = 0;
    mesh->index_count     = 0;
    mesh->vertices =
        (chunk_vertex_t*)malloc(mesh->vertex_capacity * sizeof(chunk_vertex_t));
    mesh->indices = (GLuint*)malloc(mesh->vertex_capacity * sizeof(GLuint));
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->eao);
}

int chunk_mesh_push_face(chunk_mesh_t* mesh, const uint8_t face_x, const uint16_t face_y,
                         const uint8_t face_z, bool face_vertices[4][3],
                         const float uv_offset_x, const float uv_offset_y,
                         const float uv_size, const uint8_t light_level) {
    /* Checking if vertices and indices array need to be reallocated.
     * To add a face, we need 4 new vertices. And for each face added,
     * we need 6 indices each. */
    if (mesh->vertex_count + 4 > mesh->vertex_capacity) {
        mesh->vertex_capacity *= 2;
        void* new_space =
            realloc(mesh->vertices, mesh->vertex_capacity * sizeof(chunk_vertex_t));
        if (new_space == NULL) {
            free(mesh->vertices);
            mesh->vertices = NULL;
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
            mesh->indices = NULL;
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
        mesh->vertices[mesh->vertex_count++] = chunk_vertex_pack(
            face_x + face_vertices[i][0], face_y + face_vertices[i][1],
            face_z + face_vertices[i][2], uvs[i][0], uvs[i][1], light_level);
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

static uint32_t pack_chunk_coordinate(const uint8_t arg_x, const uint16_t arg_y,
                                      const uint8_t arg_z) {
    return ((uint32_t)arg_x << 13) | ((uint32_t)arg_z << 8) | (uint32_t)arg_y;
}

static uint8_t unpack_chunk_x(const uint32_t arg_coord) {
    return (uint8_t)(arg_coord >> 13);
}

static uint8_t unpack_chunk_z(const uint32_t arg_coord) {
    return (uint8_t)((arg_coord >> 8) & 0x1F);
}

static uint16_t unpack_chunk_y(const uint32_t arg_coord) {
    return (uint16_t)(arg_coord & 0xFF);
}

void chunk_propagate_light(chunk_t* target_chunk, const chunk_neighbours_t adj_neighbours,
                           uint32_t* light_queue) {
    printf("Propagating light for chunk (%f, %f)\n", target_chunk->position[0],
           target_chunk->position[2]);

    /* Reset all light values */
    memset(target_chunk->light, 0, sizeof(target_chunk->light));

    int32_t q_head = 0;
    int32_t q_tail = 0;

    /* Handling sunlight. These loop interate on all of the columns of blocks, top to
     * bottom. It sets light to the maximum level in the queue for each block of air it
     * hits and stops once it hits one block. This means that all block above will be
     * DAY_LIGHT and every block beneath will be 0. Same process but we now check for
     * light sources and mark these blocks as MAX_LIGHT in the BFS queue. */
    for (uint8_t ix = 0; ix < CHUNK_SIZE_XZ; ++ix) {
        for (uint8_t iz = 0; iz < CHUNK_SIZE_XZ; ++iz) {
            // Sunlight: Start from top and go down until we hit an obstruction
            for (uint16_t iy = CHUNK_SIZE_Y - 1; iy > 0; --iy) {
                if (target_chunk->blocks[ix][iy][iz] != BLOCK_AIR) { break; }
                target_chunk->light[ix][iy][iz] = DAY_LIGHT;
                light_queue[q_tail++]           = pack_chunk_coordinate(ix, iy, iz);
            }

            // Emissive blocks
            for (uint16_t iy = 0; iy < CHUNK_SIZE_Y; ++iy) {
                if (is_light_block(target_chunk->blocks[ix][iy][iz])) {
                    target_chunk->light[ix][iy][iz] = MAX_LIGHT;
                    light_queue[q_tail++]           = pack_chunk_coordinate(ix, iy, iz);
                }
            }
        }
    }

    /* Checking for neighbour edges light levels. We have 4 edges to check, north,
     * south, east and west. If a light level of the neighbouring chunk edge is higher
     * than the one on the side of the current chunk, take the neighbour value and
     * remove 1 level. Then, we add this light level to the queue of our current chunk
     * so it can bleed afterwards. */
    for (uint16_t y_coord = 0; y_coord < CHUNK_SIZE_Y; ++y_coord) {
        for (uint8_t pos = 0; pos < CHUNK_SIZE_XZ; ++pos) {
            // From North neighbor (+Z)
            if (adj_neighbours.north &&
                /* If the neighbour edge block has a light value > LIGHT_FALLOFF */
                adj_neighbours.north->light[pos][y_coord][0] > LIGHT_FALLOFF) {
                const uint8_t val_north =
                    adj_neighbours.north->light[pos][y_coord][0] - LIGHT_FALLOFF;
                /* If the neighbour value diminished is greater than the current light
                 * level of the current chunk edge block, update the chunk edge light
                 * level */
                if (val_north > target_chunk->light[pos][y_coord][CHUNK_SIZE_XZ - 1]) {
                    target_chunk->light[pos][y_coord][CHUNK_SIZE_XZ - 1] = val_north;
                    /* Update the queue with the current light level so it can propagate
                     * after */
                    light_queue[q_tail++] =
                        pack_chunk_coordinate(pos, y_coord, CHUNK_SIZE_XZ - 1);
                }
            }

            // From South neighbor (-Z)
            if (adj_neighbours.south &&
                adj_neighbours.south->light[pos][y_coord][CHUNK_SIZE_XZ - 1] >
                    LIGHT_FALLOFF) {
                const uint8_t val_south =
                    adj_neighbours.south->light[pos][y_coord][CHUNK_SIZE_XZ - 1] -
                    LIGHT_FALLOFF;
                if (val_south > target_chunk->light[pos][y_coord][0]) {
                    target_chunk->light[pos][y_coord][0] = val_south;

                    light_queue[q_tail++] = pack_chunk_coordinate(pos, y_coord, 0);
                }
            }

            // From East neighbor (+X)
            if (adj_neighbours.east &&
                adj_neighbours.east->light[0][y_coord][pos] > LIGHT_FALLOFF) {
                const uint8_t val_east =
                    adj_neighbours.east->light[0][y_coord][pos] - LIGHT_FALLOFF;
                if (val_east > target_chunk->light[CHUNK_SIZE_XZ - 1][y_coord][pos]) {
                    target_chunk->light[CHUNK_SIZE_XZ - 1][y_coord][pos] = val_east;
                    light_queue[q_tail++] =
                        pack_chunk_coordinate(CHUNK_SIZE_XZ - 1, y_coord, pos);
                }
            }

            // From West neighbor (-X)
            if (adj_neighbours.west &&
                adj_neighbours.west->light[CHUNK_SIZE_XZ - 1][y_coord][pos] >
                    LIGHT_FALLOFF) {
                const uint8_t val_west =
                    adj_neighbours.west->light[CHUNK_SIZE_XZ - 1][y_coord][pos] -
                    LIGHT_FALLOFF;
                if (val_west > target_chunk->light[0][y_coord][pos]) {
                    target_chunk->light[0][y_coord][pos] = val_west;

                    light_queue[q_tail++] = pack_chunk_coordinate(0, y_coord, pos);
                }
            }
        }
    }

    /* This is the propagation loop. We have a tail at the end of the array and a head
     * at 0. */
    while (q_head < q_tail) {
        /* Taking the next coordinates of the light value in the queue */
        const uint32_t current_packed = light_queue[q_head++];

        /* Getting the light value at these coordinates */
        const uint8_t cur_x   = unpack_chunk_x(current_packed);
        const uint16_t cur_y  = unpack_chunk_y(current_packed);
        const uint8_t cur_z   = unpack_chunk_z(current_packed);
        const uint8_t cur_val = target_chunk->light[cur_x][cur_y][cur_z];

        /* Do not spread further if the light is at the minimum level */
        if (cur_val <= LIGHT_FALLOFF) { continue; };
        const uint8_t spread_val = cur_val - LIGHT_FALLOFF;

        /* Looking in all six directions from the light sources, one by one */
        for (uint8_t di = 0; di < 6; ++di) {
            const int8_t off_x[] = {1, -1, 0, 0, 0, 0};
            const int8_t off_y[] = {0, 0, 1, -1, 0, 0};
            const int8_t off_z[] = {0, 0, 0, 0, 1, -1};

            const int16_t nei_x = (int16_t)(cur_x + off_x[di]);
            const int16_t nei_y = (int16_t)(cur_y + off_y[di]);
            const int16_t nei_z = (int16_t)(cur_z + off_z[di]);

            /* Do not propagate light into neighbouring chunks, the other chunks will
             * do that propagation themselves */
            if (nei_x < 0 || nei_x >= CHUNK_SIZE_XZ || nei_y < 0 ||
                nei_y >= CHUNK_SIZE_Y || nei_z < 0 || nei_z >= CHUNK_SIZE_XZ) {
                continue;
            }

            /* Propagation condition: Only into air and only if it makes the neighbor
             * brighter */
            if (target_chunk->blocks[nei_x][nei_y][nei_z] == BLOCK_AIR) {
                if (target_chunk->light[nei_x][nei_y][nei_z] < spread_val) {
                    target_chunk->light[nei_x][nei_y][nei_z] = spread_val;
                    light_queue[q_tail++]                    = pack_chunk_coordinate(
                        (uint8_t)nei_x, (uint16_t)nei_y, (uint8_t)nei_z);
                }
            }
        }
    }
}

uint32_t chunk_vertex_pack(const uint8_t vertex_x, const uint16_t vertex_y,
                           const uint8_t vertex_z, const float uv_u, const float uv_v,
                           const uint8_t light_level) {
    const uint32_t u_idx = (uint32_t)roundf(uv_u / tile_offset);
    const uint32_t v_idx = (uint32_t)roundf(uv_v / tile_offset);
    // clang-format off
    return  (vertex_x & 0x1FU)            |
            ((vertex_z & 0x1FU)   << 5U)  |
            ((vertex_y & 0x3FFU)  << 10U) |
            (u_idx << 20U)                |
            (v_idx << 23U)                |
            ((light_level & 0xFU) << 26U);
    // clang-format on
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

/**
 * @brief Helper function to get the light level at the edge of a neighbour, if the
 * neighbour exists (loaded).
 * @param chunk Pointer to current chunk to look in.
 * @param neighbor Pointer to neighbour chunk to look in
 * @param ne_x Coordinates of the neighbour block to loot at.
 * @param ne_y Coordinates of the neighbour block to loot at.
 * @param ne_z Coordinates of the neighbour block to loot at.
 * @param cur_x Coordinates of the current block to loot at.
 * @param cur_y Coordinates of the current block to loot at.
 * @param cur_z Coordinates of the current block to loot at.
 * @return Light level at the neighbour block or current block if neighbour is not loaded.
 */
static uint8_t get_neighbor_light(const chunk_t* chunk, const chunk_t* neighbor,
                                  const int ne_x, const int ne_y, const int ne_z,
                                  const int cur_x, const int cur_y, const int cur_z) {
    return neighbor ? neighbor->light[ne_x][ne_y][ne_z]
                    : chunk->light[cur_x][cur_y][cur_z];
}

/**
 * @brief Check if a block is BLOCK_AIR.
 * @param chunk Pointer to chunk to check in.
 * @param block_x X chunk-local coordinate of the block.
 * @param block_y Y chunk-local coordinate of the block.
 * @param block_z Z chunk-local coordinate of the block.
 * @return True if the block is air, false otherwise.
 */
static bool block_is_air(const chunk_t* chunk, const int block_x, const int block_y,
                         const int block_z) {
    return chunk->blocks[block_x][block_y][block_z] == (uint8_t)BLOCK_AIR;
}

/**
 * @brief Helper function to push a face into the chunk mesh.
 * @param mesh Pointer to the chunk mesh to modify.
 * @param block_x X chunk-local coordinate of the block.
 * @param block_y Y chunk-local coordinate of the block.
 * @param block_z Z chunk-local coordinate of the block.
 * @param face Face to push, represented as a 4x3 array of booleans.
 * @param tex_uv Texture coordinates for the face.
 * @param light_factor Light factor to apply to the face.
 * @param light Light level to use for the face.
 * @return Number of vertices pushed into the mesh.
 */
static int push_face(chunk_mesh_t* mesh, const uint8_t block_x, const uint16_t block_y,
                     const uint8_t block_z, bool face[4][3], const uv_t tex_uv,
                     const float light_factor, const uint8_t light) {
    return chunk_mesh_push_face(mesh, block_x, block_y, block_z, face, tex_uv.u, tex_uv.v,
                                tile_offset, (uint8_t)(light_factor * (float)light));
}

int chunk_build_mesh(const chunk_t* chunk, chunk_mesh_t* mesh,
                     const chunk_neighbours_t neighbors, uint32_t* light_queue) {
    mesh->vertex_count = 0;
    mesh->index_count  = 0;

    /* Propagate light through the chunk */
    chunk_propagate_light((chunk_t*)chunk, neighbors, light_queue);

    for (uint8_t block_x = 0; block_x < CHUNK_SIZE_XZ; ++block_x) {
        for (uint16_t block_y = 0; block_y < CHUNK_SIZE_Y; ++block_y) {
            for (uint8_t block_z = 0; block_z < CHUNK_SIZE_XZ; ++block_z) {
                const block_type_t block = chunk->blocks[block_x][block_y][block_z];
                if (block == BLOCK_AIR) { continue; }

                const block_uv_t block_uv = block_uvs[block];

                // Front (north, +z)
                if (block_z == CHUNK_SIZE_XZ - 1) {
                    if (!neighbors.north ||
                        block_is_air(neighbors.north, block_x, block_y, 0)) {
                        if (push_face(mesh, block_x, block_y, block_z, face_front,
                                      block_uv.front, LIGHT_FRONT,
                                      get_neighbor_light(chunk, neighbors.north, block_x,
                                                         block_y, 0, block_x, block_y,
                                                         block_z))) {
                            return -1;
                        }
                    }
                } else if (block_is_air(chunk, block_x, block_y, block_z + 1)) {
                    if (push_face(mesh, block_x, block_y, block_z, face_front,
                                  block_uv.front, LIGHT_FRONT,
                                  chunk->light[block_x][block_y][block_z + 1])) {
                        return -1;
                    }
                }

                // Back (south, -z)
                if (block_z == 0) {
                    if (!neighbors.south || block_is_air(neighbors.south, block_x,
                                                         block_y, CHUNK_SIZE_XZ - 1)) {
                        if (push_face(mesh, block_x, block_y, block_z, face_back,
                                      block_uv.back, LIGHT_BACK,
                                      get_neighbor_light(chunk, neighbors.south, block_x,
                                                         block_y, CHUNK_SIZE_XZ - 1,
                                                         block_x, block_y, block_z))) {
                            return -1;
                        }
                    }
                } else if (block_is_air(chunk, block_x, block_y, block_z - 1)) {
                    if (push_face(mesh, block_x, block_y, block_z, face_back,
                                  block_uv.back, LIGHT_BACK,
                                  chunk->light[block_x][block_y][block_z - 1])) {
                        return -1;
                    }
                }

                // Top (+y)
                if (block_y == CHUNK_SIZE_Y - 1 ||
                    block_is_air(chunk, block_x, block_y + 1, block_z)) {
                    if (push_face(mesh, block_x, block_y, block_z, face_top, block_uv.top,
                                  LIGHT_TOP,
                                  block_y == CHUNK_SIZE_Y - 1
                                      ? MAX_LIGHT
                                      : chunk->light[block_x][block_y + 1][block_z])) {
                        return -1;
                    }
                }

                // Bottom (-y)
                if (block_y == 0 || block_is_air(chunk, block_x, block_y - 1, block_z)) {
                    if (push_face(mesh, block_x, block_y, block_z, face_bottom,
                                  block_uv.bottom, LIGHT_BOTTOM,
                                  block_y == 0
                                      ? 0
                                      : chunk->light[block_x][block_y - 1][block_z])) {
                        return -1;
                    }
                }

                // Right (east, +x)
                if (block_x == CHUNK_SIZE_XZ - 1) {
                    if (!neighbors.east ||
                        block_is_air(neighbors.east, 0, block_y, block_z)) {
                        if (push_face(mesh, block_x, block_y, block_z, face_right,
                                      block_uv.right, LIGHT_RIGHT,
                                      get_neighbor_light(chunk, neighbors.east, 0,
                                                         block_y, block_z, block_x,
                                                         block_y, block_z))) {
                            return -1;
                        }
                    }
                } else if (block_is_air(chunk, block_x + 1, block_y, block_z)) {
                    if (push_face(mesh, block_x, block_y, block_z, face_right,
                                  block_uv.right, LIGHT_RIGHT,
                                  chunk->light[block_x + 1][block_y][block_z])) {
                        return -1;
                    }
                }

                // Left (west, -x)
                if (block_x == 0) {
                    if (!neighbors.west || block_is_air(neighbors.west, CHUNK_SIZE_XZ - 1,
                                                        block_y, block_z)) {
                        if (push_face(mesh, block_x, block_y, block_z, face_left,
                                      block_uv.left, LIGHT_LEFT,
                                      get_neighbor_light(
                                          chunk, neighbors.west, CHUNK_SIZE_XZ - 1,
                                          block_y, block_z, block_x, block_y, block_z))) {
                            return -1;
                        }
                    }
                } else if (block_is_air(chunk, block_x - 1, block_y, block_z)) {
                    if (push_face(mesh, block_x, block_y, block_z, face_left,
                                  block_uv.left, LIGHT_LEFT,
                                  chunk->light[block_x - 1][block_y][block_z])) {
                        return -1;
                    }
                }
            }
        }
    }

    chunk_mesh_upload(mesh);
    return 0;
}

void chunk_mesh_destroy(chunk_mesh_t* mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    mesh->vertices        = NULL;
    mesh->indices         = NULL;
    mesh->vertex_count    = 0;
    mesh->index_count     = 0;
    mesh->vertex_capacity = 0;
    mesh->index_capacity  = 0;
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
