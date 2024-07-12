#ifndef WORLD_H
#define WORLD_H

#include "ds.h"
#include "skeewb.h"


typedef struct{
    int32_t x, y, z;
}block_pos_t;

typedef struct{
    uint16_t type;
    uint16_t state;
}block_t;

typedef struct{
    block_t blocks[16][16][16];
    block_pos_t offset;
}chunk_t;

typedef struct{
    
}block_type_t;


typedef struct{
    block_t (*block_at)(block_pos_t pos);
    chunk_t (*chunk_at)(block_pos_t pos);
    
    void    (*block_property_register)(string_t name);
    bool    (*block_property_get_boolean)(string_t name, uint16_t type);
    int32_t (*block_property_get_integer)(string_t name, uint16_t type);
    float   (*block_property_get_float)  (string_t name, uint16_t type);
    string_t(*block_property_get_string) (string_t name, uint16_t type);
}world_interface_t;





#endif
