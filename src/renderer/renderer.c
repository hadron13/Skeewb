#include "renderer.h"
#include <SDL2/SDL.h>

#define MODULE "Renderer"
#include "../skeewb.h"

SDL_Window *window;
SDL_Renderer *renderer;

void start(core_interface_t *core);
void loop(core_interface_t *core);
void quit(core_interface_t *core);


module_desc_t load(core_interface_t *core){
    if(SDL_Init(SDL_INIT_EVERYTHING)){
        core->console_log(ERROR, "could not start SDL, SDL error %s", SDL_GetError());
        core->quit(-1);
    }

    core->event_listen("start", (event_callback_t)start);
    core->event_listen("loop", (event_callback_t)loop);
    core->event_listen("quit", (event_callback_t)quit);
    



    return (module_desc_t){
        .modid = MODULE,
        .version = {0, 0, 1},
        .interface = NULL,
    };
}

void start(core_interface_t *core){
    config_t width_config = core->config_get("width");
    config_t height_config = core->config_get("height");
    
    int width = (width_config.type == INTEGER)? width_config.value.integer : 800;
    int height = (height_config.type == INTEGER)? height_config.value.integer : 600;

    window = SDL_CreateWindow("Skeewb", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
    if(!window){
        core->console_log(ERROR, "could not create a window, SDL error: %s", SDL_GetError());
        core->quit(-1);
    }

    core->console_log(INFO, "window created with success");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}


void loop(core_interface_t *core){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT){
            core->quit(0);
        }
    }
    SDL_SetRenderDrawColor(renderer, 50, 0, 50, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void quit(core_interface_t *core){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

