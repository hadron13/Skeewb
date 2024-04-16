#include"crane.h"
#include<locale.h>
 

#ifndef RPATH
#   ifdef UNIX
#   define RPATH " -Wl,-rpath,'$ORIGIN'/../libs/ "
#   else 
#   define RPATH
#endif
#endif 

#ifndef LIB 
#   define LIB " -Lsrc"_"libs"_"SDL"_"build "
#endif 

#ifndef INCLUDE
#   define INCLUDE " -Isrc"_"libs"_"SDL"_"include -Isrc"_"libs -Isrc"_"libs"_"glad"_"include"
#endif 


#ifndef CFLAGS
#   define CFLAGS LIB INCLUDE " -g " 
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
    string_temp_t temp = list_init(string_t);

    make_directory(str("build"));
    make_directory(str("intermediates"));
    make_directory(str("build"_"mods"));
    make_directory(str("build"_"mods"_"renderer"));
    make_directory(str("build"_"mods"_"renderer"_"shaders"));
    make_directory(str("build"_"mods"_"libs"));

    if(system("cmake" SILENCE) == 0 && !file_exists(str("build"_"mods"_"libs"_ SDL_DYLIB))){

#       ifdef UNIX 
        system("cmake -S src" _ "libs" _ "SDL" _ " -B src" _ "libs" _ "SDL" _ "build " SILENCE);
        system("cmake --build src" _ "libs" _ "SDL" _ "build " SILENCE);
        move(str("src/libs/SDL/build/libSDL3.so.0.0.0"), str("build/mods/libs/libSDL3.so.0"));    
#       elif defined(WINDOWS)
        system("cmake -S src\\libs\\SDL -B src\\libs\\SDL\\build -DCMAKE_TOOLCHAIN_FILE=build-scripts\\cmake-toolchain-mingw64-x86_64.cmake -G\"MinGW Makefiles\"");
        system("cmake --build src\\libs\\SDL\\build " SILENCE);
        move(str("src\\libs\\SDL\\build\\SDL3.dll"), str("build\\mods\\libs\\SDL3.dll"));    
#       endif
    }
    
    compile(str("build"_"skeewb" EXEC_EXT),                      str(CFLAGS),            str("src"_"skeewb.c") );
    compile(str("intermediates"_"glad.o"),                       str("-c -fPIC" CFLAGS), str("src"_"libs"_"glad"_"src"_"glad.c"));
    compile(str("build"_"mods"_"renderer"_"renderer" DYLIB_EXT), str(RENDERER_FLAGS),    str("src"_"renderer"_"renderer.c"), str("intermediates"_"glad.o"));
    
    string_t source = str("src"_"renderer"_"shaders");
    string_t *shaders = enumerate_directory(source, false);
    
    string_t destination = str("build"_"mods"_"renderer"_"shaders");

    for(size_t i = 0; i < list_size(shaders); i++){
        copy(temp_path(&temp, source, shaders[i]), temp_path(&temp, destination, shaders[i]));
    }
    
    if(strcmp(argv[1], "run") == 0){
        crane_log(INFO, "running");
        system("build"_"skeewb" EXEC_EXT);
    }


    str_temp_free(temp);
}
