#include "world_internal.h"



core_interface_t *core;


void start(core_interface_t *);
void loop(core_interface_t *);
void quit(core_interface_t *);


module_desc_t load(core_interface_t *core){






    return (module_desc_t){
        .modid = str("world"),
        .version = {0, 0, 1},
        .interface = NULL
    };
}


