#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 vnormal;

out vec3 fragPos;
out vec2 texCoord;
out vec3 normal;

uniform mat4 mvp;
uniform mat4 model;


void main(){
    gl_Position = mvp * vec4(vertex, 1.0f);
    texCoord = uv;
    fragPos = vec3(model * vec4(vertex, 1.0));
    normal = mat3(transpose(inverse(model))) * vnormal; 
}
