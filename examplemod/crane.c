#include "crane.h"

// mandatory flags for any mod
#define CFLAGS "-shared -fPIC"

int main(int argc, char **argv){
    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    }
    rebuild(argc, argv);

    string_arena_t temp = list_init(string_t);

    make_directory(str("build"));
    make_directory(temp_path(&temp, "build", "examplemod"));


    compile(temp_path(&temp, "build", "examplemod", "examplemod.so"), str(CFLAGS), temp_path(&temp, "src", "examplemod.c"));

}
