#ifndef WORLD_FOG_H
#define WORLD_FOG_H

#include "game_state.h"

/**
 * @brief Reload the fog values based on the render distance in the configuration.
 * @param shader Pointer to the shader where the values will get sent.
 * @param game_state State of the game at the moment
 */
void reload_fog(const game_state_t* game_state);

#endif // WORLD_FOG_H