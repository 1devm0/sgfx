#include "./engine.h"
//#define GLAD_GLES2_IMPLEMENTATION
//#include "deps/GLAD/gles2.h"

#ifndef __EMSCRIPTEN__
    #define GLAD_GL_IMPLEMENTATION
    #include "../engine/deps/gl.h"
#endif

u32 sgfx_update_timer(sgfx_timer_t * t, f64 ms_frame_time) {
    t -> count += ms_frame_time;
    if (t -> count > 1000) {
        t -> actual_time += 1;
        t -> count = 0;
    }
    return t -> actual_time;
}

f64 sgfx_get_dt(sgfx_frame_timer_t * t, u32 desired_fps) {
    // us to find the relative delta time in a platform-specific unit
    t -> relative_dt = ((f64) (SDL_GetPerformanceCounter() - t -> hd_last_step) / (f64) SDL_GetPerformanceFrequency());
    t -> ms_dt = t -> relative_dt * 1000; // Converting it into ms delay
    t -> special_dt_ratio = t -> relative_dt * desired_fps; // Our special DT ratio
    // printf("%f\n", props -> special_dt_ratio);
    t -> hd_last_step  = SDL_GetPerformanceCounter();
    return t -> special_dt_ratio;
}

sgfx_program_t * sgfx_init(sgfx_init_info_t * i) {
    i32 initializers = SDL_Init(SDL_INIT_EVERYTHING) + IMG_Init(IMG_INIT_PNG) + TTF_Init() + Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    if ((initializers - IMG_INIT_PNG) != 0 && SDL_GetError()[0]) {
        clog("Failed to initialize SDL2: %s", SDL_GetError());
    }       

    sgfx_program_t * p = calloc(1, sizeof(sgfx_program_t));

        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    p -> win = SDL_CreateWindow(i -> title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, i -> size[0], i -> size[1], i -> win_flags);
    SDL_SetWindowInputFocus(p -> win);

    huh("SDL Window: %p", p -> win);


    p -> i_w = i -> size[0];    
    p -> i_h = i -> size[1];    

    p -> running = 1;

    huh("Current Game Stage: %d", p -> game_stage);

    return p; 
}

void sgfx_rm(sgfx_program_t * p) {
    SDL_DestroyWindow(p -> win);
    free(p);
    Mix_CloseAudio();
    SDL_Quit();
}

void sgfx_set_px_in_surface(SDL_Surface * s, i32 x, i32 y, SDL_Color color) {
    SDL_LockSurface(s);
    u08 * pixels = s -> pixels;
    pixels[y * s -> pitch + x * s -> format -> BytesPerPixel + 0] = color.r;
    pixels[y * s -> pitch + x * s -> format -> BytesPerPixel + 1] = color.g;             
    pixels[y * s -> pitch + x * s -> format -> BytesPerPixel + 2] = color.b;
    // format changes from platforms abvoe is for mac
    // original one in engine.c

    //u08 * px = pixels + y * surf -> pitch + x;
    //*px = SDL_MapRGB(surf -> format, r, g, b);
    SDL_UnlockSurface(s);
}

void sgfx_get_px_in_surface(SDL_Surface * s, i32 x, i32 y, SDL_Color * color) {
    SDL_LockSurface(s);
    u08 * pixels = s -> pixels;
    color -> r = pixels[y * s -> pitch + x * s -> format -> BytesPerPixel + 0];
    color -> g = pixels[y * s -> pitch + x * s -> format -> BytesPerPixel + 1];             
    color -> b = pixels[y * s -> pitch + x * s -> format -> BytesPerPixel + 2];
    color -> a = 255;
    // format changes from platforms abvoe is for mac
    // original one in engine.c

    //u08 * px = pixels + y * surf -> pitch + x;
    //*px = SDL_MapRGB(surf -> format, r, g, b);
    SDL_UnlockSurface(s);
}


void sgfx_mk_swe_renderer(sgfx_swe_renderer_t * r, u32 w, u32 h) {
    r -> s = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0); 
    r -> r = SDL_CreateSoftwareRenderer(r -> s);
}

void * sgfx_get_swe_renderer_px_data(sgfx_swe_renderer_t * r) {
    return r -> s -> pixels;
}

void sgfx_start_swe_render(sgfx_swe_renderer_t * r, u08 c_r, u08 c_g, u08 c_b, u08 c_a) {
    SDL_SetRenderDrawColor(r -> r, c_r, c_g, c_b, c_a);
    SDL_RenderClear(r -> r);
}

void sgfx_end_swe_render(sgfx_swe_renderer_t * r) {
    SDL_RenderPresent(r -> r);
}

void sgfx_upload_swe_renderer_to_win(sgfx_swe_renderer_t * r, SDL_Window * w) {
    SDL_BlitScaled(r -> s, NULL, SDL_GetWindowSurface(w), NULL);
    SDL_UpdateWindowSurface(w);
}

i08 sgfx_log_ogl_call(const char * func, const char * file, i32 line) {
    GLenum err;
    while ((err = glGetError())) {
        clog("[OpenGL Error]: %s %s, ERR CODE: %d, LINE NO: %d", func, file, err, line);
        return 0;
    }
    return 1;
}



void sgfx_mk_ogl_ctx(SDL_Window * w) {
    #ifndef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        #ifdef __APPLE__
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_COMPATIBILITY, GL_TRUE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, GL_TRUE);
        #endif
    #endif
    SDL_GL_CreateContext(w);

    #ifndef __EMSCRIPTEN__
        gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    #endif
    // Enabling some OpenGL blending
    sgfx_call_gl(glEnable(GL_BLEND));
    sgfx_call_gl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


}

u32 sgfx_mk_ogl_texture(u32 texture_slot) {
    u32 id;
    sgfx_call_gl(glGenTextures(1, &id));
    sgfx_bind_ogl_texture(id, texture_slot);
    // :slot = tex_id;

    // GL_LINEAR
    sgfx_call_gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    sgfx_call_gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    // GL_CLAMP_TO_EDGE
    sgfx_call_gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    sgfx_call_gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    return id;
}

void sgfx_rm_ogl_texture(u32 id) {
    sgfx_call_gl(glDeleteTextures(1, &id));
}

void sgfx_bind_ogl_texture(u32 id, u32 texture_slot) {
    // Using only one texture slot (0)
    sgfx_call_gl(glActiveTexture(GL_TEXTURE0 + texture_slot));
    sgfx_call_gl(glBindTexture(GL_TEXTURE_2D, id));
}

void sgfx_unbind_ogl_texture() {
    sgfx_call_gl(glBindTexture(GL_TEXTURE_2D, 0));
}

void sgfx_set_ogl_texture_data(u32 w, u32 h, void * px_data) {
    // sgfx_bind_ogl_texture(id);
    // sgfx_call_gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data));
    sgfx_call_gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data));
    // sgfx_call_gl(glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, px_data));
    // sgfx_unbind_ogl_texture();
} 

void sgfx_compile_shader(const char * src, const i32 len, u32 id) {
    i32 result = 0;
    i32 info_sgfx_log_len = 0;

    // sgfx_call_gl(glShaderSource(id, 1, &str_src, NULL));
    sgfx_call_gl(glShaderSource(id, 1, &src, &len));
    sgfx_call_gl(glCompileShader(id));

    sgfx_call_gl(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

    if (result != GL_TRUE) {
        sgfx_call_gl(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_sgfx_log_len));
        char * err = calloc(info_sgfx_log_len + 1, sizeof(char));
        // sgfx_call_gl(glGetShaderInfoLog(id, info_sgfx_log_len, NULL, &err[0]));
        sgfx_call_gl(glGetShaderInfoLog(id, info_sgfx_log_len, &info_sgfx_log_len, err));
        clog("Shader Compilation Error: %s", err);
    
    }
}

u32 sgfx_link_shader(u32 vs_id, u32 fs_id) {
    u32 id = glCreateProgram();
    sgfx_call_gl(glAttachShader(id, vs_id));
    sgfx_call_gl(glAttachShader(id, fs_id));
    sgfx_call_gl(glLinkProgram(id));

    i32 result = 0;
    i32 info_sgfx_log_len = 0;

    sgfx_call_gl(glGetProgramiv(id, GL_LINK_STATUS, &result));

    if (result != GL_TRUE) {
        sgfx_call_gl(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_sgfx_log_len));
        char * err = calloc(info_sgfx_log_len + 1, sizeof(char));
        sgfx_call_gl(glGetProgramInfoLog(id, info_sgfx_log_len, NULL, &err[0]));
        clog("Shader Linkage Error: %s", err);
    }

    sgfx_call_gl(glDetachShader(id, vs_id));
    sgfx_call_gl(glDetachShader(id, fs_id));

    sgfx_rm_shader(vs_id);
    sgfx_rm_shader(fs_id);
    sgfx_call_gl(glUseProgram(id));
    return id;
}


u32 sgfx_mk_shader(const char * vs_src, const u32 vs_len, const char * fs_src, const u32 fs_len) {
    u32 ids[2];
    ids[0] = glCreateShader(GL_VERTEX_SHADER);
    ids[1] = glCreateShader(GL_FRAGMENT_SHADER);
 
    sgfx_compile_shader(vs_src, vs_len, ids[0]);
    sgfx_compile_shader(fs_src, fs_len, ids[1]);
    
    u32 id = sgfx_link_shader(ids[0], ids[1]);
    return id; 
}

void sgfx_bind_shader(u32 id) {
    sgfx_call_gl(glUseProgram(id));
}

void sgfx_unbind_shader() {
    sgfx_call_gl(glUseProgram(0));
}

void sgfx_rm_shader(u32 id) {
    sgfx_call_gl(glDeleteShader(id));
}

// used to do projection on ccpu but results weird with projectioon matrix


void sgfx_bind_renderer(sgfx_ogl_renderer_t * renderer) {
    sgfx_call_gl(glBindVertexArray(renderer -> vao));
    sgfx_call_gl(glBindBuffer(GL_ARRAY_BUFFER, renderer -> vbo));
    sgfx_call_gl(glUseProgram(renderer -> shader));
}

void sgfx_mk_ogl_renderer(sgfx_ogl_renderer_t * renderer, u32 w, u32 h, const char * vs, const char * fs) {
    // Vertex Array for storing the state of a vertex buffer and index buffer
    sgfx_call_gl(glGenVertexArrays(1, &renderer -> vao));
    // Vertex buffer which stores all points
    sgfx_call_gl(glGenBuffers(1, &renderer -> vbo));
    // Shader
    renderer -> shader = sgfx_mk_shader(vs, strlen(vs), fs, strlen(fs));

    sgfx_bind_renderer(renderer);
    sgfx_call_gl(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_WITH_ATTRIBUTES * sizeof(f32), NULL, GL_DYNAMIC_DRAW));

    // normalized option is always 0
    // Defining vertex attributes according to the point struct
    sgfx_enable_vertex_attribute_at(renderer -> shader, "position", 3, GL_FLOAT, 0, sizeof(sgfx_ogl_pt_t), offsetof(sgfx_ogl_pt_t, pos));
    sgfx_enable_vertex_attribute_at(renderer -> shader, "texture", 3, GL_FLOAT, 0, sizeof(sgfx_ogl_pt_t), offsetof(sgfx_ogl_pt_t, texture));

    // Points
    renderer -> point_count = 0;
    renderer -> vertices = calloc(MAX_VERTICES_WITH_ATTRIBUTES, sizeof(f32));

}

void sgfx_start_render(sgfx_ogl_renderer_t * renderer) {
    renderer -> point_count = 0;
}

// Just draws the vertex buffer up until the latest point
void sgfx_end_render(sgfx_ogl_renderer_t * renderer) {
    // each point has 6 attributes
    sgfx_bind_renderer(renderer);
    sgfx_call_gl(glBufferSubData(GL_ARRAY_BUFFER, 0, renderer -> point_count * VERTEX_ATTRIB_COUNT * sizeof(f32), renderer -> vertices));
    sgfx_call_gl(glDrawArrays(GL_TRIANGLES, 0, renderer -> point_count));

}

// IMPORTANT THAT THE ORTHOGRAPHIC PROJECTION IS ONLY APPLIED UPON SENDING THE POINTS TO OPENGL
// Sends a primitive's vertices to the GPU
sgfx_ogl_pt_t tmp_pos[36];
void sgfx_add_prim_to_ogl_renderer(sgfx_ogl_renderer_t * renderer, sgfx_ogl_prim_t * p) {
    // BE CAREFUL WITH MEMORY ALIGNMENT FOR VERTEX ATTRIBUTES
    // Allocate temporary position
    if (renderer->point_count * VERTEX_ATTRIB_COUNT == MAX_VERTICES_WITH_ATTRIBUTES) {
        sgfx_end_render(renderer);
        sgfx_start_render(renderer);
    }

    uv3_t rotation_vec = uv3_mk(p -> rotation.x, p -> rotation.y, p -> rotation.z);
    um4x4_t transform = um4x4_rotate_by_uv3(p -> rotation.w, rotation_vec);

    // transform = um4x4_scale_by_uv3(transform, p -> scale);
    transform = um4x4_translate_by_uv3(transform, p -> translation);

    // sgfx_ogl_pt_t * tmp_pos = calloc(p -> vertex_cnt, sizeof(sgfx_ogl_pt_t));
    for (u32 u = 0; u < p -> vertex_cnt; u++) {
        // The original points are always going to 
        // be at center, the translation will be 
        // applied to show that an object is moving,
        // keeping the center at 0, 0 allows for 
        // easier transformations in both 2D and 3D
        tmp_pos[u] = p -> pts[u];
        tmp_pos[u].pos.x *= p -> scale.x;
        tmp_pos[u].pos.y *= p -> scale.y;
        tmp_pos[u].pos.z *= p -> scale.z;
        tmp_pos[u].pos = um4x4_mult_uv3(transform, tmp_pos[u].pos);
    }
    memcpy(renderer -> vertices + renderer -> point_count * VERTEX_ATTRIB_COUNT, tmp_pos, sizeof(sgfx_ogl_pt_t) * p -> vertex_cnt);
    // u32 old = renderer -> point_count;
    renderer -> point_count += p -> vertex_cnt;
    // sgfx_call_gl(glBufferSubData(GL_ARRAY_BUFFER, old * VERTEX_ATTRIB_COUNT * sizeof(f32), p->vertex_cnt * VERTEX_ATTRIB_COUNT * sizeof(f32), tmp_pos));
}



void sgfx_rm_ogl_renderer(sgfx_ogl_renderer_t * renderer) {
    sgfx_call_gl(glDeleteBuffers(1, &renderer -> vbo));
    sgfx_call_gl(glDeleteVertexArrays(1, &renderer -> vao));
    sgfx_call_gl(glDeleteProgram(renderer -> shader));
}

void sgfx_alloc_prim(sgfx_ogl_prim_t * p, u32 vertex_count) {
    p -> pts = calloc(vertex_count, sizeof(sgfx_ogl_pt_t));
}

void sgfx_mk_ogl_primitive(sgfx_ogl_prim_t * p, f32 * vertices, u32 vertex_count) {
    memcpy(p -> pts, vertices, vertex_count * VERTEX_ATTRIB_COUNT * sizeof(f32));
    p -> vertex_cnt += vertex_count;

    p -> rotation.x = 0;
    p -> rotation.y = 0;
    p -> rotation.z = 1;
    p -> rotation.w = 0;

    p -> scale.x = 1;
    p -> scale.y = 1;
    p -> scale.z = 1;

    p -> translation.x = 0;
    p -> translation.y = 0;
    p -> translation.z = 0;
}



// Allocate first
void sgfx_mk_textured_quad(sgfx_ogl_prim_t * p, f32 x, f32 y, f32 w, f32 h, f32 texture_slot) {
    p -> pts[0].pos.x = -0.5;
    p -> pts[0].pos.y = -0.5;
    p -> pts[0].pos.z = 0.0;
    p -> pts[0].texture.x = 0.0f;
    p -> pts[0].texture.y = 0.0f;
    p -> pts[0].texture.z = texture_slot; 

    p -> pts[1].pos.x = -0.5;
    p -> pts[1].pos.y = 0.5;
    p -> pts[1].pos.z = 0.0;
    p -> pts[1].texture.x = 0.0f;
    p -> pts[1].texture.y = 1.0f;
    p -> pts[1].texture.z = texture_slot; 

    p -> pts[2].pos.x = 0.5;
    p -> pts[2].pos.y = 0.5;
    p -> pts[2].pos.z = 0.0;
    p -> pts[2].texture.x = 1.0f;
    p -> pts[2].texture.y = 1.0f;
    p -> pts[2].texture.z = texture_slot; 

    p -> pts[3].pos.x = 0.5;
    p -> pts[3].pos.y = -0.5;
    p -> pts[3].pos.z = 0.0;
    p -> pts[3].texture.x = 1.0f;
    p -> pts[3].texture.y = 0.0f;
    p -> pts[3].texture.z = texture_slot; 

    p -> pts[4] = p -> pts[0];
    p -> pts[5] = p -> pts[2];

    p -> vertex_cnt = 6;
    p -> translation.x = x + 0.5;
    p -> translation.y = y + 0.5;

    p -> scale.x = w;
    p -> scale.y = h; 

    p -> rotation.x = 0;
    p -> rotation.y = 0;
    p -> rotation.z = 1;
    p -> rotation.w = 0;
}

void sgfx_set_2d_quad_pos(sgfx_ogl_prim_t * p, f32 x, f32 y) {
    p -> translation.x = x + p -> scale.x / 2;
    p -> translation.y = y + p -> scale.y / 2;
}

void sgfx_set_primitive_texture_slot(sgfx_ogl_prim_t * p, f32 slot) {
    for (u32 u = 0; u < p -> vertex_cnt; u++) {
        p -> pts[u].texture.z = slot;
    }
}

void sgfx_convert_phys_obj_to_ogl(uphys_obj_t * o, sgfx_ogl_prim_t * p, u32 texture_slot) {
    sgfx_mk_textured_quad(p, o -> pos.x, o -> pos.y, o -> size.x, o -> size.y, texture_slot);
}
void sgfx_mk_textured_clip_quad(sgfx_ogl_prim_t * p, f32 x, f32 y, f32 w, f32 h, f32 texture_slot, f32 t_x1, f32 t_x2) {
    p -> pts[0].pos.x = -0.5;
    p -> pts[0].pos.y = -0.5;
    p -> pts[0].pos.z = 0.0;
    p -> pts[0].texture.x = t_x1;
    p -> pts[0].texture.y = 0.0f;
    p -> pts[0].texture.z = texture_slot; 

    p -> pts[1].pos.x = -0.5;
    p -> pts[1].pos.y = 0.5;
    p -> pts[1].pos.z = 0.0;
    p -> pts[1].texture.x = t_x1;
    p -> pts[1].texture.y = 1.0f;
    p -> pts[1].texture.z = texture_slot; 

    p -> pts[2].pos.x = 0.5;
    p -> pts[2].pos.y = 0.5;
    p -> pts[2].pos.z = 0.0;
    p -> pts[2].texture.x = t_x2;
    p -> pts[2].texture.y = 1.0f;
    p -> pts[2].texture.z = texture_slot; 

    p -> pts[3].pos.x = 0.5;
    p -> pts[3].pos.y = -0.5;
    p -> pts[3].pos.z = 0.0;
    p -> pts[3].texture.x = t_x2;
    p -> pts[3].texture.y = 0.0f;
    p -> pts[3].texture.z = texture_slot; 

    p -> pts[4] = p -> pts[0];
    p -> pts[5] = p -> pts[2];

    p -> vertex_cnt = 6;
    p -> translation.x = x + 0.5;
    p -> translation.y = y + 0.5;

    p -> scale.x = w;
    p -> scale.y = h; 

    p -> rotation.x = 0;
    p -> rotation.y = 0;
    p -> rotation.z = 1;
    p -> rotation.w = 0;
}




CVEC_IMPL(uv3_t, uv3);
CVEC_IMPL(sgfx_ogl_pt_t, sogl_pt);

sgfx_ogl_pt_t * sgfx_load_obj_file(const char * path, u32 vertex_count) {
    FILE * f = fopen(path, "r");
    sgfx_obj_vertex_data_t * vdata = calloc(1, sizeof(sgfx_obj_vertex_data_t));
    uvec_uv3_mk(&vdata -> vertices, vertex_count);
    uvec_uv3_mk(&vdata -> uvs, vertex_count);
    uvec_u32_mk(&vdata -> vertex_indices, vertex_count);
    uvec_u32_mk(&vdata -> uv_indices, vertex_count);
    uvec_sogl_pt_t pts;
    uvec_sogl_pt_mk(&pts, vertex_count);
    u32 curr = 0;
    while (1) {
        char head[128];
        i32 res = fscanf(f, "%s", head);
        if (res == EOF) break;
        if (strcmp(head, "v") == 0) {
            uv3_t vertex;
            fscanf(f, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            uvec_uv3_add(&vdata -> vertices, vertex);
        } else if (strcmp(head, "vt") == 0) {
            uv3_t uv;
            fscanf(f, "%f %f\n", &uv.x, &uv.y);
            uv.z = 0;
            uvec_uv3_add(&vdata -> uvs, uv);
        } else if (strcmp(head, "f") == 0) {
            u32 v_index[3], uv_index[3], n_index[3];            
            i32 matches = fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n",  &v_index[0], &uv_index[0], &n_index[0], &v_index[1], &uv_index[1], &n_index[1], &v_index[2], &uv_index[2], &n_index[2] );
            if (matches != 9) {  printf("error\n"); break; }
            uvec_u32_add(&vdata -> vertex_indices, v_index[0]);
            uvec_u32_add(&vdata -> vertex_indices, v_index[1]);
            uvec_u32_add(&vdata -> vertex_indices, v_index[2]);
            uvec_u32_add(&vdata -> uv_indices, uv_index[0]);
            uvec_u32_add(&vdata -> uv_indices, uv_index[1]);
            uvec_u32_add(&vdata -> uv_indices, uv_index[2]);

        } else {
            char stupidbuff[100];
            fgets(stupidbuff, 100, f);
        }
    }
    for (u32 u = 0; u < vdata -> vertex_indices.used; u++) {
        u32 vertex_index = vdata -> vertex_indices.e[u];
        u32 uv_index = vdata -> uv_indices.e[u];

        uv3_t vertex = vdata -> vertices.e[vertex_index - 1];
        uv3_t tex  = vdata -> uvs.e[vertex_index - 1];

        printf("\n\n%f %f %f %f %f %f\n", vertex.x, vertex.y, vertex.z, tex.x, tex.y, tex.z);

        sgfx_ogl_pt_t point;
        point.pos = vertex;
        point.texture = tex;
        uvec_sogl_pt_add(&pts, point);
    }

    printf("data\n");
    fclose(f);
    return pts.e;
}

