#include "./game.h"
#include "../engine/engine.h"

sgfx_ogl_renderer_t renderer;

sgfx_swe_renderer_t swe_renderer; 

const char * vs = 
    SHADER_VERSION_STR
    "precision lowp float;\n"
    "in vec3 position;    \n"
    "in vec3 texture;    \n"
    "out vec3 texture_details;    \n"
    "uniform mat4 projection;    \n"
    "void main() {               \n"
    "   gl_Position = projection * vec4(position.xyz, 1.0);  \n"
    "   texture_details = texture; \n"
    "}";

const char * fs = 
    SHADER_VERSION_STR
    "precision lowp float;\n"
    "uniform sampler2D texture_slots[8];\n"
    "in vec3 texture_details;    \n"
    "out vec4 frag_col;\n"
    "void main() {                                \n"
    "   int slot = int(texture_details.z); \n "
"       if (slot == 0) { frag_col = texture(texture_slots[0], texture_details.xy); } \n" 
"       if (slot == 1) { frag_col = texture(texture_slots[1], texture_details.xy); } \n" 
"       if (slot == 2) { frag_col = texture(texture_slots[2], texture_details.xy); } \n" 
"       if (slot == 3) { frag_col = texture(texture_slots[3], texture_details.xy); } \n" 
"       if (slot == 4) { frag_col = texture(texture_slots[4], texture_details.xy); } \n" 
"       if (slot == 5) { frag_col = texture(texture_slots[5], texture_details.xy); } \n" 
"       if (slot == 6) { frag_col = texture(texture_slots[6], texture_details.xy); } \n" 
"       if (slot == 7) { frag_col = texture(texture_slots[7], texture_details.xy); } \n" 
    "   frag_col.a = 1.0; \n"
    "}";

f32 cube_data[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f

};
// implement index buffers
// implement obj loading
sgfx_ogl_prim_t p;

void game_init(game_t * g) {
    i32 screen_w = 480 * 2; // 1280
    i32 screen_h = 480 * 2; // 960
    { // Creating window & renderer(s)
        g -> s = calloc(1, sizeof(game_state_t));
        g -> s -> info = (sgfx_init_info_t) { 
            "Personal Project", { screen_w, screen_h }, SDL_WINDOW_OPENGL 
        };
        
        g -> p = sgfx_init(&g -> s -> info);
        
        sgfx_mk_ogl_ctx(g -> p -> win);
        glEnable(GL_DEPTH_TEST);   
        huh("[OpenGL Version] %s", glGetString(GL_VERSION));   

    }

    sgfx_mk_ogl_renderer(&g -> s -> renderer, screen_w, screen_h, vs, fs);

    i32 active_texture_slot = glGetUniformLocation(g -> s -> renderer.shader, "texture_slots");
    i32 slots[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(active_texture_slot, 8, slots);

    sgfx_mk_swe_renderer(&g -> s -> textures[0], 100, 100);
        sgfx_start_swe_render(&g -> s -> textures[0], 255, 25, 255, 255);
        for (u32 u = 0; u < 100; u++) {
            for (u32 n = 0; n < 100; n++) {
                SDL_SetSurfaceBlendMode(g -> s -> textures[0].s, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(g -> s -> textures[0].r, u, n, 128 - n, 100);
                SDL_RenderDrawPoint(g -> s -> textures[0].r, n, u);
            }
        }
    sgfx_end_swe_render(&g -> s -> textures[0]);

    sgfx_mk_ogl_texture(0);
    sgfx_set_ogl_texture_data(100, 100, g -> s -> textures[0].s -> pixels);

    g -> s -> projection = um4x4_perspective_projection(45, screen_w, screen_h, 0.1f, 100);
    for (u32 u = 0; u < 4; u++) {
        for (u32 n = 0; n < 4; n++) {
            printf("%f ", g -> s -> projection.elems[u][n]);
        }
        printf("\n");
    }
    i32 loc = glGetUniformLocation(g -> s -> renderer.shader, "projection");
    glUniformMatrix4fv(loc, 1, 0, &g -> s -> projection.elems[0][0]);

    g -> s -> shape = calloc(1, sizeof(sgfx_ogl_prim_t));
    sgfx_alloc_prim(g -> s -> shape, 18); 
    sgfx_mk_ogl_primitive(g -> s -> shape, cube_data, 18);
    g -> s -> shape -> pts = sgfx_load_obj_file("res/pyramid.obj", 18); 
    /* sgfx_alloc_prim(g -> s -> shape, 36); 
    sgfx_mk_ogl_primitive(g -> s -> shape, cube_data, 36);
    g -> s -> shape -> pts = load_obj_file("cube.obj"); */
}

void game_run(void * _g) {
    game_t * g = (game_t *) _g;

    // sgfx_handle_viewport_aspect_ratio(-1, g -> p -> win);

    // sgfx_handle_events(game -> p, game -> s);
    // sgfx_handle_events(game -> p, game -> s);
    while (SDL_PollEvent(&g -> p -> e)) {
        switch (g -> p -> e.type) {
            case SDL_QUIT:
                g -> p-> running = 0;
                break;

            default:
                break;
        }
    }
    g -> p -> keys_pressed = SDL_GetKeyboardState(NULL);
    if (g -> p -> keys_pressed[SDL_SCANCODE_W]) {
        g -> s -> shape -> translation.z += 0.05;
    } 
    if (g -> p -> keys_pressed[SDL_SCANCODE_S]) {
        g -> s -> shape -> translation.z -= 0.05;
    }

    if (g -> p -> keys_pressed[SDL_SCANCODE_A]) {
        g -> s -> shape -> translation.x += 0.05;
    } 
    if (g -> p -> keys_pressed[SDL_SCANCODE_D]) {
        g -> s -> shape -> translation.x -= 0.05;
    }

    g -> s -> rotation += 0.01;
    sgfx_get_dt(&g -> p -> fps, 60);
    sgfx_update_timer(&g -> s -> timer, g -> p -> fps.ms_dt); 
    if (g -> s -> timer.actual_time == 1) {
        ulogger_log(LOG_DBG, "Frame Time (ms): %f", g -> p -> fps.ms_dt); 
        g -> s -> timer.actual_time = 0;
        g -> s -> timer.count = 0;
    }

    // Rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0.0f, 0.01f, 0.1f, 1.0f);
    // glDrawArrays(GL_TRIANGLES, 0, 6);

    sgfx_start_render(&g -> s -> renderer);
    
    g -> s -> shape -> scale.x = 2;
    g -> s -> shape -> rotation = uv4_mk(2, 0, 0, g -> s -> rotation);
    sgfx_add_prim_to_ogl_renderer(&g -> s -> renderer, g -> s -> shape);

    g -> s -> shape -> translation.x += 5;
    sgfx_add_prim_to_ogl_renderer(&g -> s -> renderer, g -> s -> shape);
    g -> s -> shape -> translation.x -= 5;

    sgfx_end_render(&g -> s -> renderer);

    SDL_GL_SwapWindow(g -> p -> win);
    // SDL_UpdateWindowSurface(game -> p -> win);
    if (g -> p -> fps.ms_dt < 16) {
        SDL_Delay(16 - g -> p -> fps.ms_dt);
    }
}
void game_close(game_t * g) {
    free(g -> s);
    sgfx_rm(g -> p);   
}

