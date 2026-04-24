/**
 * @file chunk.h
 * @authors Da-shou
 * @brief Header file containing all structs and functions used to create, build and
 * display chunks in a game world. Chunks are first made of an 3-dimensional array of
 * blocks, which are then compiled into a single mesh that is rendered at once by the
 * GPU. This allows for much faster rendering as the GPU doesn't need to draw each
 * block and has to check for much less face culling when two block face each other.
 */

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
 * sent to the GPU.
 */
typedef uint32_t chunk_vertex_t;

/**
 * @brief Represents the mesh of a chunk. This is then rendered by OpenGL
 * in a single call to draw all elements.
 */
typedef struct {
    chunk_vertex_t* vertices; /**< Contains all vertices of the chunk. */
    unsigned int* indices;    /**< Contains all indices to be reused when drawing */
    size_t vertex_count;      /**< Size of the vertices array */
    size_t index_count;       /**< Size of the indices array */
    size_t vertex_capacity;   /**< Capacity of the indices array */
    size_t index_capacity;    /**< Capacity of the indices array */
    GLuint vao; /**< ID of the Vertex Array Obejct used to draw this chunk */
    GLuint vbo; /**< ID of the Vertex Buffer Object used to draw this chunk */
    GLuint eao; /**< ID of the Element Buffer Array used to draw this chunk */
} chunk_mesh_t;

/**
 * @brief A chunk stores CHUNK_SIZE_XZ * 2 * CHUNK_SIZE_Y blocks. This
 * allows us to create a big mesh made out of these blocks and render it
 * with one call instead of multiple calls.
 */
typedef struct {
    uint8_t blocks[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]; /**< 3-dimensional
    array storing the id of each block (is it air, grass, stone, etc..) Each position
    correspond to the local position of the block in the chunk. */

    uint8_t light[CHUNK_SIZE_XZ][CHUNK_SIZE_Y][CHUNK_SIZE_XZ]; /**< 3-dimensional
    array storing the light level of each block. Each position
    correspond to the local position of the block in the chunk. */

    chunk_mesh_t mesh;  /** Informations about the 3D mesh of the chunk. */
    vec3 position;      /**< Position of the chunk in world-space coordinates. */
    bool modified;      /**< True if chunk has been modified and will be saved. */
    bool needs_rebuild; /**< True if the chunk should be rebuilt. */
    bool needs_light_rebuild; /**< True if chunk light sources have changed and will be
    rebuilt. */
    bool ready;         /**< True if chunk is ready to draw. */
} chunk_t;

/**
 * @brief Represents the neighbouring chunks of a chunk. This is used
 * to not render the face of blocks that face each other while being
 * on different chunks.
 */
typedef struct {
    chunk_t* west;  /**< Pointer to the chunk at x-1 */
    chunk_t* east;  /**< Pointer to the chunk at x+1 */
    chunk_t* south; /**< Pointer to the chunk at z-1 */
    chunk_t* north; /**< Pointer to the chunk at z+1 */
} chunk_neighbours_t;

/**
 * @brief Initializes a chunk to be full of air.
 * @param chunk Pointer to the chunk that will be initalized.
 * @param position Where the chunk will be positioned.
 */
void chunk_init(chunk_t* chunk, vec3 position);

/**
 * @brief Initalizes the chunk with a base size and allocates memory for
 * the vertices and indices arrays.
 * @param mesh Pointer to the chunk mesh that will be initailized.
 */
void chunk_mesh_init(chunk_mesh_t* mesh);

/**
 * @brief Pushes a face into a chunk mesh. We iterate through every cube in
 * the mesh. Everytime a face faces air, that face is added to the chunk
 * mesh to get rendered.
 * @param mesh Pointer to the mesh of the chunk.
 * @param face_x, face_y, face_z Location of the face within the chunk.
 * @param face_vertices 4 corners of the face
 * @param uv_offset_x, uv_offset_y Texture atlas offset
 * @param uv_size Size of one tile in the atlas.
 * @param light_level
 * @return 0 if successful, -1 if memory allocation failed
 */
int chunk_mesh_push_face(chunk_mesh_t* mesh, uint8_t face_x, uint16_t face_y,
                         uint8_t face_z, bool face_vertices[4][3], float uv_offset_x,
                         float uv_offset_y, float uv_size, uint8_t light_level);
/**
 * @brief Builds a mesh and pushes it to the GPU based on the block array
 * of the chunk. Will analyse the chunk, push all found faces to the mesh
 * object. Also attaches the light level of the face to each vertex.
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
 * @brief Calculate the light levels for each blocks in the chunk. Checks if there is any
 * light-emitting block in the chunk and propagates the light to the other blocks.
 * Algorithm used is BFS Flood filling.
 * @param chunk Pointer to the chunk to calculate the light for.
 * @param neighbours Neighbours of the chunk to calculate for.
 */
void chunk_propagate_light(chunk_t* chunk, chunk_neighbours_t neighbours);

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
void chunk_draw(chunk_t* chunk, const shader_t* shader, const material_t* atlas);

/**
 * @brief Destroys the allocated memory used for a chunk mesh.
 * @param mesh Pointer to the chunk mesh struct to be freed. */
void chunk_mesh_destroy(chunk_mesh_t* mesh);

/**
 * Packs the 6 values (that in total make 21 bytes of data) we need for
 * each vertex into a uint32_t to lighten the buffer data. Last 2 bits are left empty.
 * @param vertex_x X coordinate of the vertex in 3D space. (5 bits [0-16])
 * @param vertex_y Y coordinate of the vertex in 3D space. (5 bits [0-16])
 * @param vertex_z Z coordinate of the vertex in 3D space. (10 bits [0-512])
 * @param uv_u Texture coordinate U. (3 bits (0, 1, 2, or 3 divided by 4.))
 * @param uv_v Texture coordinate V. (3 bits (0, 1, 2, or 3 divided by 4.))
 * @param light_level Light level of the vertex. (4 bits [0-15])
 * @return Packed vertex data for efficient storage and transmission.
 */
uint32_t chunk_vertex_pack(uint8_t vertex_x, uint16_t vertex_y, uint8_t vertex_z,
                           float uv_u, float uv_v, uint8_t light_level);

/**
 * @brief Destroys the chunk struct storing the cubes infos.
 * @param chunk Pointer to the chunk struct to be freed. */
void chunk_destroy(chunk_t* chunk);

#endif
