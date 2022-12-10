#version 410 core

layout(vertices = 4) out;
uniform vec3 camPos;

flat in vec3 colorES[];
in vec3 posES[];
in vec2 texCoordES[];
in vec3 nES[];
in vec2 quvES[];
in vec2 qvertES[];
in vec4 glposES[];

flat out vec3 color;
out vec3 pos;
out vec2 texCoord;
out vec3 n;
out vec2 quv;
out vec2 qvert;
out vec4 glpos;

void main(){
    texCoord[gl_InvocationID] = texCoordES[gl_InvocationID];
    n[gl_InvocationID] = nES[gl_InvocationID];
    color[gl_InvocationID] = colorES[gl_InvocationID];
    pos[gl_InvocationID] = posES[gl_InvocationID];
}