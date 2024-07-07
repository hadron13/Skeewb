
#include "renderer_internal.h"

#include <SDL3/SDL.h>
#include <cglm/cglm.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include <sokol_gfx.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



SDL_Window *window;
SDL_GLContext *gl_context;
core_interface_t *core;
int width = 800, height = 600;

void start(core_interface_t *);
void loop(core_interface_t *);
void quit(core_interface_t *);


void sokol_logger(const char* tag, uint32_t log_level, uint32_t log_item, const char* message, uint32_t line_nr, const char* filename, void* user_data){

    const char *displayed_tag = (tag)? tag : "";
    const char *displayed_message = (message)? message : "";

    if(filename){
        core->console_log_(log_level, "[\033[34msokol_gfx | %s:\033[35m%d\033[0m] [%s][id: %d] %s", filename, line_nr, displayed_tag, log_item, displayed_message);
    }else{
        core->console_log_(log_level, "[\033[34msokol_gfx : \033[35m%d\033[0m] [%s][id: %d] %s", line_nr, displayed_tag, log_item, displayed_message);
    }
}


module_desc_t load(core_interface_t *core_interface){
    core = core_interface;
    core->console_log(INFO, "starting renderer");

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS)){
        core->console_log(ERROR, "could not start SDL, SDL error %s", SDL_GetError());
        core->quit(-1);
    }

    core->console_log(INFO, "telecoteco");

    core->event_listen(str("start"), (event_callback_t)start);
    core->event_listen(str("loop"), (event_callback_t)loop);
    core->event_listen(str("quit"), (event_callback_t)quit);

    return (module_desc_t){
        .modid = str("renderer"),
        .version = {0, 0, 1},
        .interface = NULL,
    };
}

sg_image texture;
sg_sampler sampler;
sg_pipeline pipeline;
sg_bindings binding;
sg_pass_action pass_action;

typedef struct{
    mat4 model, view, projection;
}mvp_t;

void start(core_interface_t *core){
    config_t width_config = core->config_get(str("width"));
    config_t height_config = core->config_get(str("height"));
    
    width = (width_config.type == TYPE_INTEGER && width_config.value.integer > 100)? width_config.value.integer : 800;
    height = (height_config.type == TYPE_INTEGER && height_config.value.integer > 100)? height_config.value.integer : 600;

    window = SDL_CreateWindow("Skeewb", width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if(!window){
        core->console_log(ERROR, "could not create a window, SDL error: %s", SDL_GetError());
        core->quit(-1);
    }
    core->console_log(INFO, "window created with success");

    
    gl_context = SDL_GL_CreateContext(window);
    if(!gl_context){
        core->console_log(ERROR, "could not create an OpenGL context, SDL error: %s", SDL_GetError());
        core->quit(-1);
    }

    core->console_log(INFO, "OpenGL context created with success");

    SDL_GL_SetSwapInterval(-1);

    
    sg_setup(&(sg_desc){
        .logger = sokol_logger,
    });

    resource_t *vertex_resource = core->resource_load(str("vert"), str("renderer/shaders/test_vert.glsl"));
    resource_t *fragment_resource = core->resource_load(str("frag"), str("renderer/shaders/test_frag.glsl"));

    string_t vertex_source = core->resource_string(vertex_resource);
    string_t fragment_source = core->resource_string(fragment_resource);

    resource_t *tex_resource = core->resource_load(str("tex"), str("renderer/assets/textures/stone.png"));

    int tex_width, tex_height, tex_channels;
    unsigned char * tex_pixels = stbi_load_from_file(tex_resource->file, &tex_width, &tex_height, &tex_channels, 0);

    sg_image cube_image = sg_make_image(&(sg_image_desc){
        .width = tex_width,
        .height = tex_height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = tex_pixels,
            .size = (size_t)(tex_width * tex_height * 4),
        }
    });

    sg_sampler cube_sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
    });

    static float vertices[] = {
       //atras
       -1.0f, -1.0f, -1.0f,     1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        1.0f, -1.0f, -1.0f,     0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        1.0f,  1.0f, -1.0f,     0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,     1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        //frente
       -1.0f, -1.0f,  1.0f,     0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f, -1.0f,  1.0f,     1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f,  1.0f,  1.0f,     1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
       -1.0f,  1.0f,  1.0f,     0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        //esquerda 
       -1.0f, -1.0f, -1.0f,     0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
       -1.0f,  1.0f, -1.0f,     0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
       -1.0f,  1.0f,  1.0f,     1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
       -1.0f, -1.0f,  1.0f,     1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        //direita
        1.0f, -1.0f, -1.0f,     1.0f, 1.0f,   1.0f, 0.0f, 0.0f, 
        1.0f,  1.0f, -1.0f,     1.0f, 0.0f,   1.0f, 0.0f, 0.0f, 
        1.0f,  1.0f,  1.0f,     0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 
        1.0f, -1.0f,  1.0f,     0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 
        //baixo
       -1.0f, -1.0f, -1.0f,     1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
       -1.0f, -1.0f,  1.0f,     1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        1.0f, -1.0f,  1.0f,     0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        1.0f, -1.0f, -1.0f,     0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        //cima
       -1.0f,  1.0f, -1.0f,     0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
       -1.0f,  1.0f,  1.0f,     0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        1.0f,  1.0f,  1.0f,     1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, -1.0f,     1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    };
 
    // index data
    static uint16_t elements[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };

    sg_buffer cube_vertex = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_VERTEXBUFFER, 
        .usage = SG_USAGE_IMMUTABLE,
        .data = SG_RANGE(vertices) 
    });
    
    sg_buffer cube_elements = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER, 
        .usage = SG_USAGE_IMMUTABLE,
        .data = SG_RANGE(elements) 
    });
    
    

    pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(&(sg_shader_desc){
            .vs ={
                .source = vertex_source.cstr,
                .uniform_blocks[0] = {
                    .size = sizeof(mat4)*2,
                    .uniforms = {
                        [0] = {.name = "mvp", .type = SG_UNIFORMTYPE_MAT4},
                        [1] = {.name = "model", .type = SG_UNIFORMTYPE_MAT4},
                    }
                }
            }, 
            .fs = {
                .source = fragment_source.cstr,
                .images[0] = {.used = true, .image_type = SG_IMAGETYPE_2D, .sample_type = SG_IMAGESAMPLETYPE_FLOAT},
                .samplers[0] = {.used = true, .sampler_type = SG_SAMPLERTYPE_FILTERING},
                .image_sampler_pairs[0] = {.used = true, .image_slot = 0, .sampler_slot = 0, .glsl_name = "texture1"}
            },
        }),
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT2,
                [2].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
    });

    binding = (sg_bindings){
        .vertex_buffers[0] = cube_vertex, 
        .index_buffer = cube_elements,
        .fs = {
            .images[0] = cube_image,
            .samplers[0] = cube_sampler,
        }
    };

    pass_action = (sg_pass_action) {
        .colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.0f, 0.0f, 0.0f, 1.0f } }
    };
}


void loop(core_interface_t *core){
    SDL_Event event;
    static bool dragging = false;
    static int xrot = 0, yrot = 0;

    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_EVENT_QUIT:
                core->quit(0); 
            case SDL_EVENT_WINDOW_RESIZED:
                SDL_GetWindowSize(window, &width, &height); 
                break; 

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                dragging = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                dragging = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if(dragging){
                    xrot += (int)event.motion.xrel;
                    yrot += (int)event.motion.yrel;
                }
                break;
        }
    }
    
    

    struct {
        mat4 mvp;
        mat4 model;
    } uniforms;
    
    mat4 view, projection;
    glm_mat4_identity(uniforms.model);
    glm_mat4_identity(view);
    glm_mat4_identity(projection);


    float t = ((float)SDL_GetTicks()/1000.0f);

    glm_perspective(glm_rad(90), (float)width/(float)height, 0.1f, 100.0f, projection);
    glm_lookat((vec3){0.0f, 0, 0.0f}, (vec3){0.0f, 0.0f, -1.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);


    glm_translate(uniforms.model, (vec3){0.0f, 0.0f, -3.0f});
    glm_rotate(uniforms.model, glm_rad(xrot), (vec3){0.0f, 0.01f, 0.0f});
    glm_rotate(uniforms.model, glm_rad(yrot), (vec3){0.01f, 0.0f, 0.0f});


    glm_mat4_mul(projection, view, uniforms.mvp);
    glm_mat4_mul(uniforms.mvp, uniforms.model, uniforms.mvp);

    sg_begin_pass(&(sg_pass){ .action = pass_action, .swapchain = (sg_swapchain){.width = width, .height = height}});
    sg_apply_pipeline(pipeline);
    sg_apply_bindings(&binding);


    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(uniforms));
    sg_draw(0, 36, 1);
    sg_end_pass();
    sg_commit();


    SDL_GL_SwapWindow(window);
}

void quit(core_interface_t *core){
    sg_shutdown();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
