#include"crane.h"

//////////////////////////////////////////////
//                                          // 
// //===\\ ||==\\      /\    ||\  || ====== //
// ||      ||   ||    //\\   ||\\ || ||     //
// ||      ||==\\    //  \\  || \\|| ||===  //
// ||      ||   \\  //====\\ ||  \\| ||     //
// \\===// ||   || //      \\||   || ====== //
//                                          // 
//      Cheap Shellscript held by Tape      //
//                                          //
//////////////////////////////////////////////

#ifndef RPATH
#   ifdef UNIX
#   define RPATH " -Wl,-rpath,'$ORIGIN'/. "
#   define ODIN_RPATH "-extra-linker-flags:\"-rpath '$ORIGIN/.'\""
#   else 
#   define RPATH
#   define ODIN_RPATH 
#endif
#endif 

#ifndef LIB 
#   define LIB " -Lskeewb/libs/SDL/build "
#endif 

#ifndef INCLUDE
#   define INCLUDE " -Iskeewb/libs/SDL/include -Isrc/libs -Isrc/libs/cglm/include -Isrc/"
#endif 


#ifndef CORE_CFLAGS
#   ifdef WINDOWS 
#       define CORE_CFLAGS " -g -DSTACKTRACE -ldbghelp" 
#   else 
#       define CORE_CFLAGS " -g -rdynamic " 
#   endif
#endif 

#ifndef MODULE_CFLAGS
#   define MODULE_CFLAGS LIB INCLUDE" -shared -fPIC -g " RPATH
#endif

#ifndef SDL_FLAGS
#   ifdef WINDOWS 
#       define SDL_FLAGS " -lmingw32 -lSDL3"
#   else 
#       define SDL_FLAGS " -lSDL3 -lGL "  
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
#   define RENDERER_FLAGS ( MODULE_CFLAGS SDL_FLAGS )
#endif

int main(int argc, char **argv){
    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    }

    rebuild(argc, argv);
    string_temp_t temp = list_init(string_t);
    
    if(argc > 1 && strcmp(argv[1], "example") == 0){ // TODO: update this!
        crane_log(INFO, "creating example mod");

        make_directory(str("examplemod"));
        make_directory(str("examplemod/src"));
        make_directory(str("examplemod/skeewb"));

        copy(str("crane.h"), str("examplemod/crane.h"));

        FILE *crane_c = fopen("examplemod/crane.c", "w");
        string_t code = str("#define MODULE \"Example\""
                            "#include\"crane.h\"\n\n"
                            "int main(int argc, char **argv){\n\t"
                            "if(!system(0))exit(-1);\n\n\t"
                            "rebuild(argc, argv);\n\t"
                            "make_directory(str(\"build\"));\n\t" 
                            "make_directory(str(\"build/mods\"));\n\t" 
                            "make_directory(str(\"build/mods/examplemod\"));\n\t" 
                            "compile(str(\"build/mods/examplemod/examplemod" DYLIB_EXT "\"), str(\"-fPIC -shared -I.\"), str(\"src/examplemod.c\"));\n}"); 
        
        fwrite(code.cstr, code.length, 1, crane_c);
        fclose(crane_c);
        
        FILE *examplemod_c = fopen("examplemod/src/examplemod.c", "w");
        code = str("#include<skeewb/skeewb.h>\n\n"
                   "module_desc_t load(core_interface_t *core){\n\t"
                   "core->console_log(INFO, \"super cool mod\");\n\n\t"
                   "return (module_desc_t){.modid = str(\"examplemod\"), .version = {0, 0, 1}};\n}"); 
        
        fwrite(code.cstr, code.length, 1, examplemod_c);
        fclose(examplemod_c);

        copy(str("skeewb/ds.h"), str("examplemod/skeewb/ds.h"));
        copy(str("skeewb/skeewb.h"), str("examplemod/skeewb/skeewb.h"));
        copy(str("skeewb/renderer/renderer.h"), str("examplemod/skeewb/renderer.h"));
       
        exit(0);
    }

    

    make_directory(str("build"));
    make_directory(str("intermediates"));
    make_directory(str("build/mods"));
    make_directory(str("build/mods/renderer"));
    make_directory(str("build/mods/renderer/assets/"));
    make_directory(str("build/mods/renderer/assets/shaders"));
    make_directory(str("build/mods/renderer/assets/textures")); 
    make_directory(str("build/mods/world"));

    
    compile(str("build/skeewb" EXEC_EXT), str(CORE_CFLAGS), str("skeewb/core/skeewb.c") );

    
    compile(str("build/mods/world/world" DYLIB_EXT), str(MODULE_CFLAGS),    
            str("skeewb/world/world.c"));
    
    //TODO: compile() but Odin
    system("odin build skeewb/renderer -out=build/mods/renderer/renderer" DYLIB_EXT " -build-mode:shared -collection:skeewb=skeewb -define:MODULE=\"Renderer\" " ODIN_RPATH);
    
    
    copy(str("skeewb/renderer/assets/shaders/*"), str("build/mods/renderer/assets/shaders/"));
    copy(str("skeewb/renderer/assets/textures/*"), str("build/mods/renderer/assets/textures/"));
    
    str_temp_free(temp);
}
