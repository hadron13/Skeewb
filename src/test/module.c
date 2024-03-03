
#define MODULE "Test"
#include "../skeewb.h"


void start(void *context);
void loop(void *context);


int load(core_interface_t *core){
    core->log(DEBUG, "Hello Module! core version %u.%u.%u", core->version.major, core->version.minor, core->version.patch);
    
    core->event_listen("start", start);
    core->event_listen("loop", loop);



    return 0;
}

void start(void *context){
    core_interface_t *core = context;
    core->log(DEBUG, "eae");
}


void loop(void *context){
    core_interface_t *core = context;
    
    core->log(DEBUG, "bomm diaaa!!!");

}



