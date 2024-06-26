
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <stdlib.h>

#define MODULE "Renderer"
#include "../skeewb.h"
#include "renderer.h"


SDL_Window *window;
SDL_GLContext *gl_context;
core_interface_t *core;

void start(core_interface_t *);
void loop(core_interface_t *);
void quit(core_interface_t *);



shader_t *renderer_shader_compile(resource_t vertex, resource_t fragment);
void      renderer_shader_use(shader_t *shader);




renderer_interface_t renderer = {
    .shader_compile = renderer_shader_compile,
    .shader_use = renderer_shader_use,
};



module_desc_t load(core_interface_t *core_interface){

    core = core_interface;
    core->console_log(INFO, "starting renderer");

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS)){
        core->console_log(ERROR, "could not start SDL, SDL error %s", SDL_GetError());
        core->quit(-1);
    }

    core->event_listen(str("start"), (event_callback_t)start);
    core->event_listen(str("loop"), (event_callback_t)loop);
    core->event_listen(str("quit"), (event_callback_t)quit);
    

    return (module_desc_t){
        .modid = str("renderer"),
        .version = {0, 0, 1},
        .interface = &renderer,
    };
}

void start(core_interface_t *core){
    config_t width_config = core->config_get(str("width"));
    config_t height_config = core->config_get(str("height"));
    
    int width = (width_config.type == INTEGER)? width_config.value.integer : 800;
    int height = (height_config.type == INTEGER)? height_config.value.integer : 600;

    window = SDL_CreateWindow("Skeewb", 1080, 720 , SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
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
    

    SDL_GL_SetSwapInterval(1);
    
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    gladLoadGL();
   
    core->console_log(INFO, "OpenGL version %s loaded", glGetString(GL_VERSION));
    core->console_log(INFO, "Vendor: %s", glGetString(GL_VENDOR));


    resource_t vertex_resource = core->resource_load(str("vert"), str("renderer/shaders/test_vert.glsl"));
    resource_t fragment_resource = core->resource_load(str("frag"), str("renderer/shaders/test_frag.glsl"));
    
    shader_t *shader = renderer_shader_compile(vertex_resource, fragment_resource);

}


void loop(core_interface_t *core){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_EVENT_QUIT){
            core->quit(0);
        }
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
}

void quit(core_interface_t *core){
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

