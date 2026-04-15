#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "chunk_store.h"
#include "../game_config.h"

#define MAX_RENDER_DISTANCE 60
#define MAX_LOADED_CHUNKS_SIZE ((2 * MAX_RENDER_DISTANCE) + 1)

/**
 * @brief Signature for a chunk terrain generator.
 * @param chunk The chunk to fill with blocks.
 * @param world_cx World-space chunk X coordinate.
 * @param world_cz World-space chunk Z coordinate.
 * @param userdata Arbitrary params passed through from world_t.
 */
typedef void (*chunk_generator_t)(chunk_t* chunk, int world_cx,
                                  int world_cz, const void* userdata);

/** @brief Struct containing all the data needed to draw the entire world.
 * Uses a toroidal buffer: a fixed (2*RENDER_DISTANCE+1)^2 grid of chunk
 * slots centered on the player. slot_cx/slot_cz record which world chunk
 * each slot holds (INT_MIN = empty). world_get_chunk performs an O(1)
 * lookup. */
typedef struct {
    chunk_t chunks[MAX_LOADED_CHUNKS_SIZE][MAX_LOADED_CHUNKS_SIZE];
    int slot_cx[MAX_LOADED_CHUNKS_SIZE][MAX_LOADED_CHUNKS_SIZE];
    int slot_cz[MAX_LOADED_CHUNKS_SIZE][MAX_LOADED_CHUNKS_SIZE];
    int last_player_cx;
    int last_player_cz;
    uint8_t render_distance;
    chunk_generator_t generate;
    void* generator_data;
    chunk_store_t chunk_store;
} world_t;

/**
 * @brief Initializes all chunk slots and marks them as empty.
 * Set world.generate and world.generator_data before calling
 * world_update for the first time.
 * @param world Pointer to world structure to be initialized.
 * @param config Pointer to game configuration for render distance.
 */
void world_init(world_t* world, game_config_t* config);

/**
 * @brief Per-frame streaming update. Evicts chunks outside render distance
 * and loads new ones by calling world->generate. Call once per
 * frame before world_draw.
 * @param world      Pointer to the world.
 * @param player_pos Current player world-space position.
 */
void world_update(world_t* world, const vec3 player_pos);

/**
 * @brief O(1) chunk lookup by world-space chunk coordinates.
 * @param world Pointer to the world.
 * @param cx World-space chunk X index.
 * @param cz World-space chunk Z index.
 * @return Pointer to the chunk, or NULL if not currently loaded. */
chunk_t* world_get_chunk(world_t* world, int chunk_x, int chunk_z);

/** @brief Builds the mesh for the chunk at slot (sx, sz). */
void world_build_chunk(world_t* world, int slot_x, int slot_z);

/**
 * @brief Changes the active render distance and forces a full reload.
 * Clamped to [1, MAX_RENDER_DISTANCE]. Marks all slots empty so the
 * next world_update regenerates the correct set of chunks.
 * @param world           Pointer to the world.
 * @param render_distance New render distance in chunks.
 */
void world_set_render_distance(world_t* world, int render_distance);

/**
 * @brief Rebuilds the mesh of a modified chunk and any loaded neighbors
 * whose border geometry changed.
 * @param world   Pointer to the world.
 * @param chunk_x World-space chunk X of the modified chunk.
 * @param chunk_z World-space chunk Z of the modified chunk.
 * @param local_x Block's local X within the chunk.
 * @param local_z Block's local Z within the chunk.
 */
void world_rebuild_after_change(world_t* world, int chunk_x, int chunk_z,
                                int local_x, int local_z);
/**
 * @brief Draws all loaded chunks with frustum culling.
 * @param world   Pointer to the world to be drawn.
 * @param shader  Pointer to shader program to be used.
 * @param atlas   Pointer to atlas texture material to be used.
 * @param frustum Frustum planes for culling.
 */
void world_draw(world_t* world, const shader_t* shader, material_t* atlas,
                vec4 frustum[6]);

/**
 * @brief Frees all chunk meshes in the world.
 * @param world Pointer to the world to be destroyed. */
void world_destroy(world_t* world);

/**
 * @brief Returns true if position is within the loaded world and valid Y
 * range.
 * @param world    Pointer to the world.
 * @param position World-space position to check. */
bool world_valid_position(const world_t* world, const vec3 position);

/** @brief Parameters for world_generator_perlin. */
typedef struct {
    float scale;   /**< Large or smaller hills. */
    int sea_level; /**< Height of sea level. */
    int amplitude; /**< Maximum height of hills. */
} perlin_params_t;

/**
 * @brief Perlin-noise terrain generator. Pass a perlin_params_t* as
 * userdata. Produces stone/sand/dirt/grass columns at heights driven by 2D
 * perlin noise. */
void world_generator_perlin(chunk_t* chunk, int world_cx, int world_cz,
                            const void* userdata);

#endif // WORLD_H
