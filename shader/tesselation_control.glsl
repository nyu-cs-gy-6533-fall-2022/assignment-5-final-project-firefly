#version 410 core

layout(vertices = 4) out;
uniform vec3 camPos;

flat in vec3 color[];
in vec3 pos[];
in vec2 texCoord[];
in vec3 n[];
in vec2 quv[];
in vec2 qvert[];
in vec4 glpos[];

flat out vec3 colorES[];
out vec3 posES[];
out vec2 texCoordES[];
out vec3 nES[];
out vec2 quvES[];
out vec2 qvertES[];
out vec4 glposES[];

void main(){
    texCoordES[gl_InvocationID] = texCoord[gl_InvocationID];
    nES[gl_InvocationID] = n[gl_InvocationID];
    colorES[gl_InvocationID] = color[gl_InvocationID];
    posES[gl_InvocationID] = pos[gl_InvocationID];
}