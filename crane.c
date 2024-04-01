#include"crane.h"
#include<locale.h>


#ifndef RPATH
#   ifdef UNIX
#   define RPATH " -Wl,-rpath,'$ORIGIN'/libs/ "
#   else 
#   define RPATH
#endif
#endif 

#ifndef LIB 
#   define LIB " -Lsrc"S"libs"S"SDL"S"build"
#endif 

#ifndef INCLUDE
#   define INCLUDE " -Isrc"S"libs"S"SDL"S"include -Isrc"S"libs"
#endif 


#ifndef CFLAGS
#   define CFLAGS LIB INCLUDE " -g   " 
#endif 

#ifndef MODULE_CFLAGS
#   define MODULE_CFLAGS LIB INCLUDE" -shared -fPIC -O3 " RPATH
#endif

#ifndef SDL_FLAGS
#   ifdef WINDOWS 
#       define SDL_FLAGS " -lmingw32 -lSDL3"
#   else 
#       define SDL_FLAGS " -lSDL3 "  
#   endif
#endif

#ifndef SDL_DYLIB
#   ifdef WINDOWS 
#       define SDL_DYLIB "SDL3.dll"
#   else 
#       define SDL_DYLIB "libSDL3.so.0"  
#   endif
#endif



#ifndef RENDERER_FLAGS
#   define RENDERER_FLAGS (MODULE_CFLAGS SDL_FLAGS)
#endif

int main(int argc, char **argv){
    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    } 
    rebuild(argc, argv);
    string_arena_t arena = list_init(string_t);

    make_directory(str("build"));
    make_directory(arena_path(&arena, "build", "mods"));
    make_directory(arena_path(&arena, "build", "mods", "libs"));

    if(system("cmake" SILENCE) == 0 && !file_exists(arena_path(&arena, "build", "mods", "libs", SDL_DYLIB))){
        system("cmake -S src" S "libs" S "SDL" S    " -B src" S "libs" S "SDL" S "build " SILENCE);
        system("cmake --build src" S "libs" S "SDL" S "build " SILENCE);

#       ifdef UNIX 
        move(str("src/libs/SDL/build/libSDL3.so.0.0.0"), str("build/mods/libs/libSDL3.so.0"));    
#       elif defined(WINDOWS)
        //SDL3.dll?
        move(str("src/libs/SDL/build/SDL3.dll"), str("build/mods/libs/SDL3.dll"));    
#       endif
    }
    

    compile(arena_path(&arena, "build", "skeewb" EXEC_EXT),            str(CFLAGS),        arena_path(&arena, "src", "skeewb.c") );
    compile(arena_path(&arena, "build", "mods", "renderer" DYLIB_EXT), str(RENDERER_FLAGS), arena_path(&arena, "src", "renderer", "renderer.c"));

    
    str_arena_free(arena);
}
