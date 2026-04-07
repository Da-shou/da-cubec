#ifndef MESH_H
#define MESH_H

#include <stdint.h>

/**
 * @brief Structure containing all necessary attributes of a 3D mesh
 * @param vertices Array for the local coordinates of the mesh's vertices
 * @param texture_coordinates Array for the texture coordinates
 * @param vertex_count Number of vertex needed for this mesh
 * @param index_count Number of indices needed for this mesh 
 * @param indices Array for the indices of the reused vertices 
 * @param vao Vertex Array Object needed for the mesh
 * @param vbos Vertex Buffer Objects needed for the mesh
 * @param eao Element Array Object needed for the mesh
 * */
typedef struct {
        float* vertices;
        float* texture_coordinates;
        uint64_t vertex_count;
        uint64_t index_count;
        unsigned int* indices;
	unsigned int vao;
	unsigned int* vbos;
	unsigned int eao;
} mesh_t;

/** 
 * @brief Frees an allocated mesh
 * @param m Mesh to be freed. */
void mesh_free(mesh_t* m);

#endif
