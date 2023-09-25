#include "examples/game.h"

// Never put a while loop in emscripten 
i32 main(i32 argv, char ** args) {
    game_t game; 
    game_init(&game);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(game_run, &game, 250, 1);
#else
    while (game.p -> running) {
        game_run(&game);
    }
#endif
    game_close(&game); 
    return 0;
}
