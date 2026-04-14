#ifndef WORLD_H
#define WORLD_H

#include <chunk.h>

#define WORLD_SIZE_X 4
#define WORLD_SIZE_Z 4

/** @brief Struct containing all the data needed to draw the entire world.
 */
typedef struct {
        chunk_t chunks[WORLD_SIZE_X][WORLD_SIZE_Z];
} world_t;

/**
 * @brief Fills the chunk array of the world with the chunks and their
 * position.
 * @param world Pointer to world structure to be initalized.
 */
void world_init(world_t* world);

/**
 * Builds the meshes of all the chunks contained in the world.
 * @param world Pointer to the world to the built.
 */
void world_build(world_t* world);

/**
 * Uses a world, a shader and a texture atlas to draw all the chunks
 * on screen.
 * @param world Pointer to the world to be drawn.
 * @param shader Pointer to shader program to be used.
 * @param atlas Pointer to atlas texture material to be used.
 */
void world_draw(world_t* world, shader_t* shader, material_t* atlas);

/**
 * Frees all chunks in the world.
 * @param world Pointer to the world to be destroyed.
 */
void world_destroy(world_t* world);

/** @brief Checks if that position exists in the world
 * @param world Pointer to the world to be checked.
 * @param position Vec3 containing the position to check.
 */
bool world_valid_position(const vec3 position);

/**
 * Fills world with half filled chunks with a surface of grass, 3 layers of
 * dirt, 1 layer of sand, then stone.
 * @param world The world to be filled.
 */
void world_fill_superflat(world_t* world);

#endif //WORLD_H
