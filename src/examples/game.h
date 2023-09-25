#pragma once
#include "../engine/engine.h"

typedef struct {
    // config_file_t config;
    sgfx_init_info_t info;
    sgfx_timer_t timer;

    sgfx_swe_renderer_t textures[2];
    sgfx_ogl_renderer_t renderer;

    sgfx_ogl_prim_t * shape;

    um4x4_t projection;
    f32 rotation;
} game_state_t;

typedef struct {
    game_state_t * s;
    sgfx_program_t * p;
} game_t;

void game_init(game_t * g);
void game_run(void * _g);
void game_close(game_t * g);

