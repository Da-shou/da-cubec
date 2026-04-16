#ifndef CHUNK_H
#define CHUNK_H

#include <stddef.h>
#include <stdint.h>

#include "shader.h"
#include "material.h"

#define CHUNK_SIZE_XZ 16
#define CHUNK_SIZE_Y 512

static const size_t starting_chunk_capacity = 1024;

/**
 * @brief Each chunk will be made of a certain amount of vertices. The
 * total amount depends on the number of blocks in the chunk. The values
 * are packed into a signle uint32_t and are then unpacked in the vertex
 * shader to lighten the rendering process by lightening the buffer data
 * sent to the GPU. */
typedef uint32_t chunk_vertex_t;

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
 * @brief A chunk stores CHUNK_SIZE_XZ * 2 * CHUNK_SIZE_Y blocks. This
 * allows us to create a big mesh made out of these blocks and render it
 * with one call instead of multiple calls. */
typedef struct {
    uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ];
    chunk_mesh_t mesh;
    vec3 position;
    bool modified;
} chunk_t;

/**
 * @brief Represents the neighbouring chunks of a chunk. This is used
 * to not render the face of blocks that face each other while being
 * on different chunks.
 */
typedef struct {
    const chunk_t* west;  // x-1
    const chunk_t* east;  // x+1
    const chunk_t* south; // z-1
    const chunk_t* north; // z+1
} chunk_neighbours_t;

/**
 * @brief Initializes a chunk to be full of air.
 * @param chunk Pointer to the chunk that will be initalized.
 * @param position Where the chunk will be positioned. */
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
 * @param mesh Pointer to the mesh of the chunk.
 * @param x,y,z Location of the face within the chunk.
 * @param face_vertices 4 corners of the face
 * @param uv_offset_x, uv_offset_y Texture atlas offset
 * @param uv_size Size of one tile in the atlas.
 * @return 0 if successful, -1 if memory allocation failed
 */
int chunk_mesh_push_face(chunk_mesh_t* mesh, uint8_t face_x, uint16_t face_y,
                          uint8_t face_z, bool face_vertices[4][3],
                          float uv_offset_x, float uv_offset_y,
                          float uv_size);
/**
 * @brief Builds a mesh and pushes it to the GPU based on the block array
 * of the chunk. Will analyse the chunk, push all found faces to the mesh
 * object.
 * @param chunk Data of all the blocks in the chunk that will be analyzed.
 * @param mesh Pointer to the mesh struct that will be filled with all
 * necesarry faces and sent to the GPU.
 * @param neighbors Struct containing 4 pointers to the neighbours of the
 * chunk. The neighbours are checked when the blocks of the current chunk
 * are on the edge, so the chunk does not render a face that is facing a
 * face from a neighbouring chunk.
 * @return 0 if successful, -1 if memory allocation failed
 */
int chunk_build_mesh(const chunk_t* chunk, chunk_mesh_t* mesh,
                      chunk_neighbours_t neighbors);

/**
 * @brief Uploads a mesh to the GPU. Is meant to be used inside
 * chunk_build_mesh. Do not use individually.
 * @param mesh Pointer to the mesh that will be sent to GPU. */
void chunk_mesh_upload(const chunk_mesh_t* mesh);

/**
 * @brief Draws the mesh using the vertices and elements contained in the
 * mesh structure.
 * @param mesh Pointer to the mesh to be drawn. */
void chunk_mesh_draw(const chunk_mesh_t* mesh);

/**
 * @brief Sets the shader and material containing the texture atlas and
 * calls the drawing of the chunk mesh.
 * @param chunk Pointer to the chunk to be rendered.
 * @param shader Pointer to the shader program used.
 * @param atlas Pointer to the texture atlas used.
 * */
void chunk_draw(chunk_t* chunk, const shader_t* shader, material_t* atlas);

/**
 * @brief Destroys the allocated memory used for a chunk mesh.
 * @param mesh Pointer to the chunk mesh struct to be freed. */
void chunk_mesh_destroy(chunk_mesh_t* mesh);

/**
 * Packs the 5 values (that in total make 20 bytes of data) we need for
 * each vertex into a uint32_t to lighten the buffer data.
 * @param x X coordinate of the vertex in 3D space. (5 bits [0-16])
 * @param y Y coordinate of the vertex in 3D space. (5 bits [0-16])
 * @param z Z coordinate of the vertex in 3D space. (10 bits [0-512])
 * @param u Texture coordinate U. (3 bits (0, 1, 2, or 3 divided by 4.))
 * @param v Texture coordinate V. (3 bits (0, 1, 2, or 3 divided by 4.))
 * @return Packed vertex data for efficient storage and transmission.
 */
uint32_t chunk_vertex_pack(uint8_t vertex_x, uint16_t vertex_y, uint8_t vertex_z, float uv_u,
                           float uv_v);

/**
 * @brief Destroys the chunk struct storing the cubes infos.
 * @param chunk Pointer to the chunk struct to be freed. */
void chunk_destroy(chunk_t* chunk);

#endif
