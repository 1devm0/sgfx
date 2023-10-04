#pragma once
#include "deps/utils/utils.h" 
// #include "deps/GLAD/gles2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <sdl_gfx/SDL2_gfxPrimitives.h> 
#include <sdl_gfx/SDL2_framerate.h> 
#include <sdl_gfx/SDL2_imageFilter.h> 
#include <sdl_gfx/SDL2_gfxPrimitives_font.h> 
#include <sdl_gfx/SDL2_rotozoom.h> 
#include <signal.h>


#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
#else 
    #include "../engine/deps/gl.h"
#endif


i32 main(i32 argv, char ** args);
#define SDL_main main

/*
    ECS: https://austinmorlan.com/posts/entity_component_system/#the-component
*/

/*
    Timer
*/
typedef struct {
    f64 count;
    u32 actual_time;
} sgfx_timer_t;

// Make second-based timer
#define sgfx_mk_timer() (sgfx_timer_t) { 0, 0 }
// A time oriented timer that only updates after a second has passed
u32 sgfx_update_timer(sgfx_timer_t * t, f64 ms_frame_time);
/*
    Frame Timer
*/

typedef struct {
    u64 hd_last_step;
    f64 relative_dt;
    f64 ms_dt;
    f64 special_dt_ratio; // ratio we use to get each variable equal dspite fps, formula is desired_fps/curr_fps
} sgfx_frame_timer_t;

// Create a frame timer
#define mk_sgfx_frame_timer(t) memset((t), 0, sizeof(sgfx_frame_timer_t))
// Delta time (special ratio)
f64 sgfx_get_dt(sgfx_frame_timer_t * t, u32 desired_fps);


typedef struct {
    char * title;
    u32 size[2];
    u32 win_flags; 
} sgfx_init_info_t; 

typedef struct {
    SDL_Window * win;
    // initial size
    u32 i_w;
    u32 i_h;

    u08 running;

    SDL_Event e;
    const u08 * keys_pressed;

    u08 game_stage;
    u08 game_stage_changed;

    sgfx_frame_timer_t fps;
} sgfx_program_t;

// Initialize a basic "engine" state
sgfx_program_t * sgfx_init(sgfx_init_info_t * i);

// Delte the "engine" state
void sgfx_rm(sgfx_program_t * p);


// Set the pixel color of a certain pixel in a surface
void sgfx_set_px_in_surface(SDL_Surface * s, i32 x, i32 y, SDL_Color color);
// Get the pixel color of a certain pixel in a surface
void sgfx_get_px_in_surface(SDL_Surface * s, i32 x, i32 y, SDL_Color * color);


// Set scale of any SDL Renderer
#define sgfx_set_render_scale SDL_RenderSetScale

// TODO:
    // ui label
    // draw functions
    // animated image
    // audio
    // pointer tracking -> free

// Software renderer, simply an SDL_Surface that allows a user of the library to draw to the surface
typedef struct _sgfx_swe_renderer {
    // Surface
    SDL_Surface * s;
    // Renderer
    SDL_Renderer * r;
} sgfx_swe_renderer_t; 

// Create software renderer
void sgfx_mk_swe_renderer(sgfx_swe_renderer_t * r, u32 w, u32 h); 
// Get raw pixel data for a software renderer
void * sgfx_get_swe_renderer_px_data(sgfx_swe_renderer_t * r); 
// Clears the software renderer 
void sgfx_start_swe_render(sgfx_swe_renderer_t * r, u08 c_r, u08 c_g, u08 c_b, u08 c_a);
// Updates the whole software renderer
void sgfx_end_swe_render(sgfx_swe_renderer_t * r); 
// Uploads the software renderer data to the window
void sgfx_upload_swe_renderer_to_win(sgfx_swe_renderer_t * r, SDL_Window * w); 

////// Actual OpenGL

// OpenGL function abstraction for setting vertex attributes such as what defines a vertex (position, texture coordinates...etc...)
#define sgfx_enable_vertex_attribute_at(shader, attrib_name, elems, type, normalized, stride, offset) \
    sgfx_call_gl(glVertexAttribPointer(glGetAttribLocation(shader, attrib_name), elems, type, normalized, stride, (void *)((i64) offset))); \
    sgfx_call_gl(glEnableVertexAttribArray(glGetAttribLocation(shader, attrib_name)));


// Debug utilities for logging OpenGL errors based on where they occurred in a file 
i08 sgfx_log_ogl_call(const char * func, const char * file, i32 line);
#define dbg_brk  raise(SIGABRT)
#define cls_err() { while (glGetError() != GL_NO_ERROR); }
#define sgfx_assert_glfunc(x) if (!(x)) dbg_brk;
// This is the macro that is supposed to be used
#define sgfx_call_gl(x) cls_err(); x; sgfx_assert_glfunc(sgfx_log_ogl_call(#x, __FILE__, __LINE__)); 


// Creates an OpenGL context
void sgfx_mk_ogl_ctx(SDL_Window * w);

// Creates an OpenGL texture ID
u32 sgfx_mk_ogl_texture(u32 texture_slot);
// Deletes a shader and frees the memory associated with it
void sgfx_rm_ogl_texture(u32 id);

// Binds a texture to the OpenGL state machine
// Question: How do texture slots and binding texture work?
void sgfx_bind_ogl_texture(u32 id, u32 texture_slot);
// Unbinds a texture from OpenGL state machine 
void sgfx_unbind_ogl_texture();

// Set the pixel data in an OpenGL texture. It is expected that the pixel data is RGBA with each pixel having a depth of 32 bits
void sgfx_set_ogl_texture_data(u32 w, u32 h, void * px_data);


// Makes a shader by taking in the Vertex Shader source and Fragment shader source along with both shaders' respective lengths (number of characters)
u32 sgfx_mk_shader(const char * vs_src, const u32 vs_len, const char * fs_src, const u32 fs_len);
// Binds shader to the OpenGL state machine
void sgfx_bind_shader(u32 id);
// Unbinds shader from the OpenGL state machine
void sgfx_unbind_shader();
// Deletes a shader and therefore frees the memory allocated for a shader
void sgfx_rm_shader(u32 id);

// TODO: Handle aspect ratio and resizing properly
um4x4_t sgfx_handle_viewport_aspect_ratio(f32 target_aspect_ratio, SDL_Window * w, um4x4_t projection);


typedef struct {
    uv3_t pos;    
    uv3_t texture; // change to vec3, 3rd component being the texture slot
} sgfx_ogl_pt_t;

typedef struct {
    sgfx_ogl_pt_t * pts;
    uv3_t translation;
    uv3_t scale;
    uv4_t rotation;
    u32 vertex_cnt;
} sgfx_ogl_prim_t;

// 2k squares is more than enough or 2000 * 6
#define MAX_TRIANGLES 12e3
#define VERTEX_ATTRIB_COUNT 6
#define MAX_VERTICES_WITH_ATTRIBUTES (i32) (MAX_TRIANGLES * 3) * VERTEX_ATTRIB_COUNT // 9 for each data

typedef struct {
    u32 vbo;
    u32 vao;
    u32 shader;

    u32 point_count;
    f32 * vertices;
} sgfx_ogl_renderer_t; 

// Create renderer
void sgfx_mk_ogl_renderer(sgfx_ogl_renderer_t * renderer, u32 w, u32 h, const char * vs, const char * fs);
// Bind the VBO, VAO, Shader of a renderer
void sgfx_bind_renderer(sgfx_ogl_renderer_t * renderer);
// Start rendering
void sgfx_start_render(sgfx_ogl_renderer_t * renderer);
// Just draws the vertex buffer up until the latest point
void sgfx_end_render(sgfx_ogl_renderer_t * renderer); 
// IMPORTANT THAT THE ORTHOGRAPHIC PROJECTION IS ONLY APPLIED UPON SENDING THE POINTS TO OPENGL
// Sends a primitive's vertices to the GPU
void sgfx_add_prim_to_ogl_renderer(sgfx_ogl_renderer_t * renderer, sgfx_ogl_prim_t * p); 
// Delete Renderer
void sgfx_rm_ogl_renderer(sgfx_ogl_renderer_t * renderer);
// Copy whatever vertex data you want to, into a primitive
void sgfx_mk_ogl_primitive(sgfx_ogl_prim_t * p, f32 * vertices, u32 vertex_count);
void sgfx_alloc_prim(sgfx_ogl_prim_t * p, u32 vertex_count);
// Special Utility Functions
// Create a textured quad
void sgfx_mk_textured_quad(sgfx_ogl_prim_t * p, f32 x, f32 y, f32 w, f32 h, f32 texture_slot); 
void sgfx_set_2d_quad_pos(sgfx_ogl_prim_t * p, f32 x, f32 y);
void sgfx_set_primitive_texture_slot(sgfx_ogl_prim_t * p, f32 slot);
void sgfx_convert_phys_obj_to_ogl(uphys_obj_t * o, sgfx_ogl_prim_t * p, u32 texture_slot);
void sgfx_mk_textured_clip_quad(sgfx_ogl_prim_t * p, f32 x, f32 y, f32 w, f32 h, f32 texture_slot, f32 t_x1, f32 t_x2);
// Renderer taken from VoxelRifts' tutorial
#ifdef __EMSCRIPTEN__
    #define SHADER_VERSION_STR "#version 300 es\n" 
#else
    #define SHADER_VERSION_STR "#version 130\n" 
#endif

CVEC_DEF(uv3_t, uv3);
CVEC_DEF(sgfx_ogl_pt_t, sogl_pt);

typedef struct {
    uvec_uv3_t vertices;
    uvec_uv3_t uvs;
    uvec_u32_t vertex_indices;
    uvec_u32_t uv_indices;
} sgfx_obj_vertex_data_t;

sgfx_ogl_pt_t * sgfx_load_obj_file(const char * path, u32 vertex_count);