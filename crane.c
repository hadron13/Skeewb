#include"crane.h"

#define CFLAGS "-O3"
#define MODULE_CFLAGS "-shared -fPIC"



int main(int argc, char **argv){

    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    }
    rebuild(argc, argv);
   
    char **sources = list_init(char*);

    string_arena arena;
    
    list_push(sources, str_arena_add(arena, string_path(2, "src", "skeewb.c")) ); 
    compile(sources, CFLAGS, str_arena_add(arena, string_path(2, "build", "skeewb" EXEC_EXT)));


    sources[0] = str_arena_add(arena, string_path(3, "src", "renderer", "renderer.c")); 
    compile(sources, MODULE_CFLAGS "-lSDL2", "build/mods/renderer" DYLIB_EXT);

    list_free(sources);

    str_arena_free(arena);
}
