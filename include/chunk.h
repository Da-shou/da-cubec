#ifndef CHUNK_H
#define CHUNK_H

#include <stdlib.h>
#include <stdint.h>
#include <blocks.h>
#include <shader.h>
#include <material.h>

#define CHUNK_SIZE 16

static const size_t STARTING_CHUNK_CAPACITY = 1024;

/**
 * @brief Each chunk will be made of a certain amount of vertices. The
 * total amount depends on the number of blocks in the chunk. */
typedef struct {
    float x, y, z;
    float u, v;
} chunk_vertex_t;

/**
 * @brief Represents the mesh of a chunk. This is then rendered by OpenGL
 * in a single call to draw all elements. */
typedef struct {
    chunk_vertex_t* vertices;
    unsigned int* indices;
    size_t vertex_count;
    size_t index_count;
    size_t vertex_capacity;
    size_t index_capacity;
    unsigned int vao, vbo, eao;
} chunk_mesh_t;

/**
 * @brief A chunk stores 16x16x16 blocks. This allows us to create a big
 * mesh made out of these 4096 blocks and render it with one call instead f
 * 4096 calls. */
typedef struct {
    block_type_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    chunk_mesh_t mesh;
    vec3 position;
} chunk_t;

/** 
 * @brief Initializes a chunk to be full of air.
 * @param chunk Pointer to the chunk that will be initalized. */
void chunk_init(chunk_t* chunk, vec3 position);

/**
 * @brief Initalizes the chunk with a base size and allocates memory for
 * the vertices and indices arrays.
 * @param mesh Pointer to the chunk mesh that will be initailized. */
void chunk_mesh_init(chunk_mesh_t* mesh);

/**
 * @brief Pushes a face into a chunk mesh. We iterate through every cube in
 * the mesh. Everytime a face faces air, that face is added to the chunk
 * mesh to get rendered.
 * @param x,y,z Location of the face within the chunk.
 * @param face_vertices 4 corners of the face
 * @param uv_offset_x, uv_offset_y Texture atlas offset
 * @param uv_size Size of one tile in the atlas.*/
void chunk_mesh_push_face(chunk_mesh_t* mesh, float x, float y, float z,
                          float face_vertices[4][3], float uv_offset_x,
                          float uv_offset_y, float uv_size);
/**
 * @brief Builds a mesh and pushes it to the GPU based on the block array
 * of the chunk. Will analyse the chunk, push all found faces to the mesh
 * object.
 * @param chunk Data of all the blocks in the chunk that will be analyzed.
 * @param mesh Pointer to the mesh struct that will be filled with all
 * necesarry faces and sent to the GPU. */
void chunk_build_mesh(chunk_t* chunk, chunk_mesh_t* mesh);

/**
 * @brief Uploads a mesh to the GPU. Is meant to be used inside
 * chunk_build_mesh. Do not use individually.
 * @param mesh Pointer to the mesh that will be sent to GPU. */
void chunk_mesh_upload(chunk_mesh_t* mesh);

/**
 * @brief Draws the mesh using the vertices and elements contained in the
 * mesh structure. 
 * @param mesh Pointer to the mesh to be drawn. */
void chunk_mesh_draw(chunk_mesh_t* mesh);

/**
 * @brief Sets the shader and material containing the texture atlas and calls the drawing of the chunk mesh.
 * @param chunk Pointer to the chunk to be rendered.
 * */
void chunk_draw(chunk_t* chunk, shader_t* shader, material_t* material); 

/**
 * @brief Destroys the allocated memory used for a chunk mesh.
 * @param mesh Pointer to the chunk mesh struct to be freed. */
void chunk_mesh_destroy(chunk_mesh_t* mesh);

/**
 * @brief Destroys the chunk struct storing the cubes infos.
 * @param chunk Pointer to the chunk struct to be freed. */
void chunk_destroy(chunk_t* chunk);

#endif
