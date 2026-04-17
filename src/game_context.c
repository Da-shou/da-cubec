#include "game_context.h"

static game_context_t* s_game_context = NULL;

void set_game_context(game_context_t* game_context) {
    s_game_context = game_context;
}

game_context_t* get_game_context(void) {
    return s_game_context;
}