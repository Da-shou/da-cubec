#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include "text_renderer.h"
#include "game_state.h"

void draw_debug_info(const text_renderer_t* text_renderer,
                     const game_state_t* game_state);

#endif // DEBUG_UI_H
