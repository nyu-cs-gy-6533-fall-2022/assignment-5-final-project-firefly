#version 330 core

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
    // vec3 col = texture(tex, texCoord).rgb;
    // vec3 col = triangleColor;
    vec3 normal = normalize(n);
    // vec3 lightDir = normalize(lightPos - pos);
    // col = clamp( col * lightParams.x + 
    //     col * max(0.0, dot(normal, lightDir)) + 
    //     vec3(1.0) * pow(max(0.0, dot( normalize(camPos - pos), normalize( reflect(-lightDir, normal)))), lightParams.y),
    //     0.0, 1.0);

    // else{
    // gl_FragData[0] = texture(tex, texCoord) * vec4(color, 1.0);
    // gl_FragData[0] = vec4(heightMap, 1.0);
    gl_FragData[0] = vec4(color, 1.0);
    // gl_FragData[0] = vec4(biome(pos.y), 1.0) * vec4(color, 1.0);
    gl_FragData[1] = vec4(normal, 0);
    gl_FragData[2] = vec4(pos, 0);

    // }

    
}
