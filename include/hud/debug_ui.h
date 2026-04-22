/**
 * @file debug_ui.h
 * @authors Da-shou
 * Header file for functions used to display debug informationn on the screen.
 */

#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include "text_renderer.h"
#include "game_state.h"

/**
 * @brief Draws debug information on screen taken from the current game state.
 * @param text_renderer Pointer to the text renderer to use.
 * @param game_state Pointer to the current game state
 */
void draw_debug_info(const text_renderer_t* text_renderer,
                     const game_state_t* game_state);

#endif // DEBUG_UI_H
