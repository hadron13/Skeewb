#define MODULE "Renderer"
#include"renderer.h"
#include<glad/glad.h>

extern core_interface_t *core;

struct shader_t{
    string_t name;
    GLuint ID;
    resource_t vertex;
    resource_t fragment;
};

GLuint shader_stage_compile(string_t source, GLenum type){
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char* const*)&source.cstr, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        core->console_log(ERROR, "shader compile error: %s\n", infoLog);
        return 0;
    }
    return shader;
}

shader_t *renderer_shader_compile(string_t name, resource_t vertex, resource_t fragment){
    string_t vertex_source = core->resource_string(vertex); 
    string_t fragment_source = core->resource_string(fragment);

    core->console_log(DEBUG, "%s", vertex_source.cstr);
    core->console_log(DEBUG, "%s", fragment_source.cstr);

    GLuint vertex_shader = shader_stage_compile(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = shader_stage_compile(fragment_source, GL_FRAGMENT_SHADER); 

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        core->console_log(ERROR, "shader link error: %s\n", infoLog);
        return NULL;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return NULL;
}

void renderer_shader_use(shader_t *shader){
    glUseProgram(shader->ID);
}


