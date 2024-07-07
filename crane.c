#include"crane.h"
#include<locale.h>
 

#ifndef RPATH
#   ifdef UNIX
#   define RPATH " -Wl,-rpath,'$ORIGIN'/. "
#   else 
#   define RPATH
#endif
#endif 

#ifndef LIB 
#   define LIB " -Lsrc/libs/SDL/build "
#endif 

#ifndef INCLUDE
#   define INCLUDE " -Isrc/libs/SDL/include -Isrc/libs -Isrc/libs/cglm/include -Isrc/"
#endif 


#ifndef CFLAGS
#   ifdef WINDOWS 
#       define CFLAGS LIB INCLUDE " -g  " 
#   else 
#       define CFLAGS LIB INCLUDE " -g " 
#   endif
#endif 

#ifndef MODULE_CFLAGS
#   define MODULE_CFLAGS LIB INCLUDE" -shared -fPIC -g " RPATH
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
#   define RENDERER_FLAGS (MODULE_CFLAGS SDL_FLAGS " -lGL ")
#endif

int main(int argc, char **argv){
    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    } 
    rebuild(argc, argv);
    string_temp_t temp = list_init(string_t);
    
    if(argc > 1 && strcmp(argv[1], "rebuild") == 0){
        
    }

    make_directory(str("build"));
    make_directory(str("intermediates"));
    make_directory(str("build/mods"));
    make_directory(str("build/mods/renderer"));
    make_directory(str("build/mods/renderer/assets/"));
    make_directory(str("build/mods/renderer/assets/shaders"));
    make_directory(str("build/mods/renderer/assets/textures")); 

    if(system("cmake" SILENCE) == 0 && !file_exists(str("build/mods/renderer/" SDL_DYLIB))){
#       ifdef UNIX
        crane_log(INFO, "configuring SDL3 cmake (might take a while)...");
        if(system("cmake -S src/libs/SDL/ -B src/libs/SDL/build" SILENCE)){
            crane_log(ERROR, "could not configure SDL3 cmake");
            exit(-1);
        }
#       elif defined(WINDOWS)
        crane_log(INFO, "configuring SDL3 cmake (might take a while)...");
        if(system("cmake -S src\\libs\\SDL -B src\\libs\\SDL\\build -DCMAKE_TOOLCHAIN_FILE=build-scripts\\cmake-toolchain-mingw64-x86_64.cmake -G\"MinGW Makefiles\"")){
            crane_log(ERROR, "could not configure SDL3 cmake");
            exit(-1);
        }
#       endif

        crane_log(INFO, "building SDL3 (might take a while)...");
        if(system("cmake --build src/libs/SDL/build " SILENCE)){
            crane_log(ERROR, "could not compile SDL3");
            exit(-1);
        }

        copy(str("src/libs/SDL/build/" SDL_DYLIB), str("build/mods/renderer/" SDL_DYLIB));    
    } 
    
    compile(str("build/skeewb" EXEC_EXT), str(CFLAGS), str("src/skeewb.c") );

    compile(str("build/mods/renderer/renderer" DYLIB_EXT), str(RENDERER_FLAGS),    
            str("src/renderer/renderer.c"));
    
    
    copy(str("src/renderer/shaders/*"), str("build/mods/renderer/assets/shaders/"));
    
    if(argc > 1 && strcmp(argv[1], "run") == 0){
        crane_log(INFO, "running");
        system("build/skeewb" EXEC_EXT);
    }
    str_temp_free(temp);
}
