#include"crane.h"

#define CFLAGS "-O3"
#define MODULE_CFLAGS "-shared -fPIC"

int main(int argc, char **argv){

    if(!system(0)){
        crane_log(CRITICAL, "huh where shell");
        exit(-1);
    }
    

}
