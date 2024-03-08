#include"crane.h"

#define CFLAGS "-O3"
#define MODULE_CFLAGS "-shared -fPIC -O3"

int main(int argc, char **argv){

    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    }
    rebuild(argc, argv);  

    string_arena_t arena = list_init(string_t);
 
    make_directory(str("build"));
    make_directory(arena_path(&arena, "build", "mods"));

    compile(arena_path(&arena, "build", "skeewb" EXEC_EXT),            str(CFLAGS),        arena_path(&arena, "src", "skeewb.c") );
    compile(arena_path(&arena, "build", "mods", "renderer" DYLIB_EXT), str(MODULE_CFLAGS), arena_path(&arena, "src", "renderer", "renderer.c"));
    
    str_arena_free(arena);
}
