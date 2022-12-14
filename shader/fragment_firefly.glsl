#version 410 core

out vec4 outColor;

in vec2 texCoord;
in vec3 n;
flat in vec3 color;
// in vec3 color;
in vec3 pos;
in vec3 norms;
in vec2 quv;
in vec2 qvert;
// in vec3 heightMap;

in vec4 glpos;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform int task;

uniform sampler2D tex;


void main()
{

    vec3 normal = normalize(n);
    
    // gl_FragData[0] = texture(tex, vec2(position.x, position.z));
    float snowDistance = length(vec3(camPos.x, camPos.y - 0.5, camPos.z) - pos);

    gl_FragData[0] = vec4(color, 1/(snowDistance / 1.4)); //change snow oppacity on istance from camera
    gl_FragData[1] = vec4(normal, 0);
    gl_FragData[2] = vec4(pos, 0);
    
}
