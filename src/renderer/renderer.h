#ifndef RENDERER_H
#define RENDERER_H

#include "../ds.h"
#include "../skeewb.h"


// TODO: neat interface


typedef struct shader_t shader_t;


typedef struct{
    shader_t *(*shader_compile)(resource_t vertex, resource_t fragment);
    shader_t *(*shader_get)(string_t name);
    void      (*shader_use)(shader_t *shader);
    void     *(*shader_set_bool)(shader_t *shader, bool value);
    void     *(*shader_set_int)(shader_t *shader, int32_t value);
    void     *(*shader_set_uint)(shader_t *shader, uint32_t value);
    void     *(*shader_set_float)(shader_t *shader, float value);
    void     *(*shader_set_vec2)(shader_t *shader, float *value);
    void     *(*shader_set_vec3)(shader_t *shader, float *value);
    void     *(*shader_set_vec4)(shader_t *shader, float *value);
    void     *(*shader_set_mat2)(shader_t *shader, float *value);
    void     *(*shader_set_mat3)(shader_t *shader, float *value);
    void     *(*shader_set_mat4)(shader_t *shader, float *value);
}renderer_interface_t;










#endif
