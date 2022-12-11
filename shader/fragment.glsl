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

vec3 biome(float elevation)
{
    if (elevation <= 0.13)
    { // WATER BIOME
        // return vec3(68.0 / 255.0, 68.0 / 255.0, 120.0 / 255.0);
        return vec3(255.0 / 205.0, 0.0 / 255.0, 130.0 / 255.0);
    }
    else if (elevation < 0.2)
    { // BEACH
        // return vec3(161.0 / 255.0, 144.0 / 255.0, 120.0 / 255.0);
        return vec3(51.0 / 255.0, 95.0 / 255.0, 121.0 / 255.0);
    }
    else if (elevation < 0.3)
    { // FOREST
        // return vec3(81.0 / 255.0, 154.0 / 255.0, 78.0 / 255.0);
        return vec3(56.0 / 255.0, 94.0 / 255.0, 186.0 / 255.0);
    }
    else if (elevation < 0.4)
    { // DEEP FOREST
        // return vec3(46.0 / 255.0, 120.0 / 255.0, 88.0 / 255.0);
        return vec3(26.0 / 255.0, 58.0 / 255.0, 104.0 / 255.0);
    }
    else if (elevation <0.5){
        // return vec3(118.0 / 255.0, 162.0 / 255.0, 132.0 / 255.0);
        return vec3(87.0 / 255.0, 175.0 / 255.0, 228.0 / 255.0);
    }
    else if (elevation < 0.6)
    { // STEPPES
        // return vec3(152.0 / 255.0, 170.0 / 255.0, 123.0 / 255.0);
        return vec3(98.0 / 255.0, 194.0 / 255.0, 210.0 / 255.0);
    }
    else if (elevation < 0.8)
    { // STONE
        // return vec3(155.0 / 255.0, 172.0 / 255.0, 160.0 / 255.0);
        return vec3(114.0 / 255.0, 196.0 / 255.0, 247.0 / 255.0);
    }
    else if (elevation < 0.9)
    { // SNOW
        // return vec3(210.0 / 255.0, 226.0 / 255.0, 227.0 / 255.0);
         return vec3(179.0 / 255.0, 196.0 / 255.0, 237.0 / 255.0);
    }
    else
    { // SNOW CAP
        // return vec3(241.0 / 255.0, 252.0 / 255.0, 252.0 / 255.0);
        return vec3(255.0 / 40.0, 187.0 / 255.0, 107.0 / 255.0);
    }
}

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
    float distanceFromCam = length(pos - camPos)/3.0 * length(pos - camPos)/3.0;
    // else{
    // gl_FragData[0] = texture(tex, texCoord);
    // gl_FragData[0] =vec4(texCoord,0.0,1.0);
    // gl_FragData[0] = vec4(heightMap, 1.0);
    // gl_FragData[0] = vec4(color, 1.0);
    gl_FragData[0] = vec4(biome(pos.y), 1.0/ distanceFromCam) * vec4(color, 1.0 / distanceFromCam);
    gl_FragData[1] = vec4(normal, 0);
    gl_FragData[2] = vec4(pos, 0);

    // }

    
}
