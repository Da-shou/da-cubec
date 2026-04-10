#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#include <cglm/cglm.h>

#include <chunk.h>
#include <blocks.h>
#include <shader.h>
#include <material.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// clang-format off
/* These are all the different vertices for a face that are needed. */
static float FACE_FRONT[4][3] = {
	{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};

static float FACE_BACK[4][3] = {
	{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}};

static float FACE_LEFT[4][3] = {
	{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}};

static float FACE_RIGHT[4][3] = {
	{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}};

static float FACE_TOP[4][3] = {
	{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}};

static float FACE_BOTTOM[4][3] = {
	{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}};
//clang-format on

void chunk_init(chunk_t* chunk, vec3 position) {
        glm_vec3_copy(position, chunk->position);
        memset(chunk->blocks, 0, sizeof(chunk->blocks));
        chunk_mesh_init(&chunk->mesh);
}

void chunk_mesh_init(chunk_mesh_t* mesh) {
        mesh->vertex_capacity = STARTING_CHUNK_CAPACITY;
        mesh->index_capacity = STARTING_CHUNK_CAPACITY;
        mesh->vertex_count = 0;
        mesh->index_count = 0;
        mesh->vertices = (chunk_vertex_t*)malloc(mesh->vertex_capacity *
                                                 sizeof(chunk_vertex_t));
        mesh->indices = (unsigned int*)malloc(mesh->vertex_capacity *
                                              sizeof(unsigned int));
        glGenVertexArrays(1, &mesh->vao);
        glGenBuffers(1, &mesh->vbo);
        glGenBuffers(1, &mesh->eao);
}

void chunk_mesh_push_face(chunk_mesh_t* mesh, float x, float y, float z,
                          float face_vertices[4][3], float uv_offset_x,
                          float uv_offset_y, float uv_size) {
        /* Checking if vertices and indices array need to be reallocated.
         * To add a face, we need 4 new vertices. And for each face added,
         * we need 6 indices each. */
        if (mesh->vertex_count + 4 > mesh->vertex_capacity) {
                mesh->vertex_capacity *= 2;
                mesh->vertices =
                    realloc(mesh->vertices, mesh->vertex_capacity *
                                                sizeof(chunk_vertex_t));
        }

        if (mesh->index_count + 6 > mesh->index_capacity) {
                mesh->index_capacity *= 2;
                mesh->indices =
                    realloc(mesh->indices,
                            mesh->index_capacity * sizeof(unsigned int));
        }

        /* Storing the UV corners for this atlas tile */
        float uvs[4][2] = {
            {uv_offset_x, uv_offset_y + uv_size},
            {uv_offset_x + uv_size, uv_offset_y + uv_size},
            {uv_offset_x + uv_size, uv_offset_y},
            {uv_offset_x, uv_offset_y},
        };

        unsigned int base = mesh->vertex_count;

        /* Adding the 4 vertices needed to draw a face to the mesh.
         * The values added to x, y and z are given from static arrays
         * allowing a quick access to the necessary offset for each
         * vertex.*/
        for (uint8_t i = 0; i < 4; ++i) {
                mesh->vertices[mesh->vertex_count++] = (chunk_vertex_t) {
                    .x = x + face_vertices[i][0],
                    .y = y + face_vertices[i][1],
                    .z = z + face_vertices[i][2],
                    .u = uvs[i][0],
                    .v = uvs[i][1],
                };
        }

        /* Here we push 6 indices, which will draw two triangles, which in
         * turn will draw a single face of the chunk. */
        mesh->indices[mesh->index_count++] = base + 0;
        mesh->indices[mesh->index_count++] = base + 1;
        mesh->indices[mesh->index_count++] = base + 2;
        mesh->indices[mesh->index_count++] = base + 2;
        mesh->indices[mesh->index_count++] = base + 3;
        mesh->indices[mesh->index_count++] = base + 0;
}

void chunk_build_mesh(chunk_t* chunk, chunk_mesh_t* mesh) {
    // clang-format off
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    /* Big ass check on ALL cubes and sending each facing that face
     * BLOCK_AIR to the chunk mesh to be built. */
    for (uint8_t x = 0; x < CHUNK_SIZE; ++x) {
        for (uint8_t y = 0; y < CHUNK_SIZE; ++y) {
            for (uint8_t z = 0; z < CHUNK_SIZE; ++z) {
                uint8_t block = chunk->blocks[x][y][z];
                if (block == BLOCK_AIR) continue;

                block_uv_t uv = BLOCK_UVS[block];

                /* Checking the X,Y,Z coordinates of each block, in front
                 * and behind.*/
                if (z == CHUNK_SIZE - 1 ||
                    chunk->blocks[x][y][z + 1] == BLOCK_AIR)
                    chunk_mesh_push_face(mesh, x, y, z, FACE_FRONT,
				    uv.front.u, uv.front.v, TILE_OFFSET);
                if (z == 0 || chunk->blocks[x][y][z - 1] == BLOCK_AIR)
                    chunk_mesh_push_face(mesh, x, y, z, FACE_BACK, 
				    uv.back.u, uv.back.v, TILE_OFFSET);

                if (y == CHUNK_SIZE - 1 ||
                    chunk->blocks[x][y + 1][z] == BLOCK_AIR)
                    chunk_mesh_push_face(mesh, x, y, z, FACE_TOP,
				    uv.top.u, uv.top.v, TILE_OFFSET);
                if (y == 0 || chunk->blocks[x][y - 1][z] == BLOCK_AIR)
                    chunk_mesh_push_face(mesh, x, y, z, FACE_BOTTOM, 
				    uv.bottom.u, uv.bottom.v, TILE_OFFSET);

                if (x == CHUNK_SIZE - 1 ||
                    chunk->blocks[x + 1][y][z] == BLOCK_AIR)
                    chunk_mesh_push_face(mesh, x, y, z, FACE_RIGHT,
				    uv.right.u, uv.right.v, TILE_OFFSET);
                if (x == 0 || chunk->blocks[x - 1][y][z] == BLOCK_AIR)
                    chunk_mesh_push_face(mesh, x, y, z, FACE_LEFT,
				    uv.left.u, uv.left.v, TILE_OFFSET);
            }
        }
    }
    chunk_mesh_upload(mesh);
}
// clang-format on

void chunk_mesh_upload(chunk_mesh_t* mesh) {
        glBindVertexArray(mesh->vao);

        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->vertex_count * sizeof(chunk_vertex_t),
                     mesh->vertices, GL_DYNAMIC_DRAW);

        /* offsetof is a function giving the byte offset of a member in a
         * struct. Very useful in our case here. x is the start of the
         * first attribute and is 3 floats (x, y and z.). Then we tell
         * OpenGL that our second attribute starts at the u and will be u
         * and v (the texture coordinates on the atlas.) */
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(chunk_vertex_t),
                              (void*)offsetof(chunk_vertex_t, x));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              sizeof(chunk_vertex_t),
                              (void*)offsetof(chunk_vertex_t, u));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eao);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     mesh->index_count * sizeof(unsigned int),
                     mesh->indices, GL_DYNAMIC_DRAW);
}

void chunk_mesh_draw(chunk_mesh_t* mesh) {
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT,
                       0);
        glBindVertexArray(0);
}

void chunk_draw(chunk_t* chunk, shader_t* shader, material_t* atlas) {
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
