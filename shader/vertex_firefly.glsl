#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform int task;

uniform vec3 offsets[100];

in vec3 position;
in vec3 normal;
in vec2 UV;
in vec2 QUV;
in vec2 qVert;
// in vec3 HM;

out vec3 n;
flat out vec3 color;
// out vec3 color;
out vec3 pos;
out vec2 texCoord;
out vec3 norms;
out vec2 quv;
out vec2 qvert;
out vec4 glpos;
// out vec3 heightMap;

uniform sampler2D texPos;

float rand3D(vec3 co){
    return fract(sin(dot(co.xyz ,vec3(12.9898,78.233,144.7272))) * 43758.5453);
}

void main()
{       
        //shader program for flying lights
        //low poly coloring

        n = mat3(transpose(inverse(modelMatrix))) * normal;
        
        vec3 col = vec3(255.0/255.0, 250.0/255.0, 236.0/255.0);

        color = col;
        pos = vec3(modelMatrix * vec4(vec3(position.x, texture(texPos, texCoord).y, position.z), 1.0));

        vec3 offset = offsets[gl_InstanceID];
        gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vec3(position.x + offset.x , position.y + offset.y, position.z + offset.z), 1.0);
        // gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position , 1.0);
        // gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position.x, Noise3D(vec3(position.x,position.y, position.z), 0.5), position.z , 1.0);
        norms = normal;

        // glpos =  projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

        texCoord = UV;
        // heightMap = HM;
        quv = QUV;
        qvert = qVert;
}
