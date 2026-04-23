/**
 * @file world/fog.h
 * @authors Da-shou
 * @brief Header file for the functions designed to handle the fog calculations.
 */

#ifndef WORLD_FOG_H
#define WORLD_FOG_H

#include "game_state.h"

/**
 * @brief Reload the fog values based on the render distance in the configuration.
 * @param game_state Pointer to the state of the game
 */
void reload_fog(const game_state_t* game_state);

#endif // WORLD_FOG_H