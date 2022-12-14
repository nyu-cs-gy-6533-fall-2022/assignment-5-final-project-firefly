#version 410 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;

//multiple lighting uniforms
uniform int numSnow;
uniform vec3 snow[100];
uniform vec3 terrainOffset[9];

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

uniform sampler2D tex;

float rand3D(vec3 co){
    return fract(sin(dot(co.xyz ,vec3(12.9898,78.233,144.7272))) * 43758.5453);
}

float simple_interpolate(float a, float b, float x)
{
   return a + smoothstep(0.0,1.0,x) * (b-a);
}
float interpolatedNoise3D(float x, float y, float z)
{
    float integer_x = x - fract(x);
    float fractional_x = x - integer_x;

    float integer_y = y - fract(y);
    float fractional_y = y - integer_y;

    float integer_z = z - fract(z);
    float fractional_z = z - integer_z;

    float v1 = rand3D(vec3(integer_x, integer_y, integer_z));
    float v2 = rand3D(vec3(integer_x+1.0, integer_y, integer_z));
    float v3 = rand3D(vec3(integer_x, integer_y+1.0, integer_z));
    float v4 = rand3D(vec3(integer_x+1.0, integer_y +1.0, integer_z));

    float v5 = rand3D(vec3(integer_x, integer_y, integer_z+1.0));
    float v6 = rand3D(vec3(integer_x+1.0, integer_y, integer_z+1.0));
    float v7 = rand3D(vec3(integer_x, integer_y+1.0, integer_z+1.0));
    float v8 = rand3D(vec3(integer_x+1.0, integer_y +1.0, integer_z+1.0));

    float i1 = simple_interpolate(v1,v5, fractional_z);
    float i2 = simple_interpolate(v2,v6, fractional_z);
    float i3 = simple_interpolate(v3,v7, fractional_z);
    float i4 = simple_interpolate(v4,v8, fractional_z);

    float ii1 = simple_interpolate(i1,i2,fractional_x);
    float ii2 = simple_interpolate(i3,i4,fractional_x);

    return simple_interpolate(ii1 , ii2 , fractional_y);
}

vec3 biome(float elevation)
{
    if (elevation <= 0.1)
    { // WATER BIOME
        return vec3(68.0 / 255.0, 68.0 / 255.0, 120.0 / 255.0);
    }
    else if (elevation < 0.2)
    { // BEACH
        return vec3(161.0 / 255.0, 144.0 / 255.0, 120.0 / 255.0);
    }
    else if (elevation < 0.3)
    { // FOREST
        // return vec3(81.0 / 255.0, 154.0 / 255.0, 78.0 / 255.0);
        return vec3(56.0 / 255.0, 94.0 / 255.0, 186.0 / 255.0);
    }
    else if (elevation < 0.4)
    { // DEEP FOREST
        // return vec3(46.0 / 255.0, 120.0 / 255.0, 88.0 / 255.0);
        return vec3(87.0 / 255.0, 175.0 / 255.0, 228.0 / 255.0);
    }
    else if (elevation <0.5){
        return vec3(118.0 / 255.0, 162.0 / 255.0, 132.0 / 255.0);
    }
    else if (elevation < 0.6)
    { // STEPPES
        return vec3(152.0 / 255.0, 170.0 / 255.0, 123.0 / 255.0);
    }
    else if (elevation < 0.8)
    { // STONE
        return vec3(155.0 / 255.0, 172.0 / 255.0, 160.0 / 255.0);
    }
    else if (elevation < 0.9)
    { // SNOW
        // return vec3(210.0 / 255.0, 226.0 / 255.0, 227.0 / 255.0);
        return vec3(179.0 / 255.0, 196.0 / 255.0, 237.0 / 255.0);
    }
    else
    { // SNOW CAP
        return vec3(241.0 / 255.0, 252.0 / 255.0, 252.0 / 255.0);
    }
}

float Noise3D(vec3 coord, float wavelength)
{
   return interpolatedNoise3D(coord.x/wavelength, coord.y/wavelength, coord.z/wavelength);
}

vec3 bhong(vec3 lPos, vec3 amb, vec3 lCol, vec3 specCol, vec3 lParams){

    //low poly coloring
    // opposite offset for multiple light sources
    n = mat3(transpose(inverse(modelMatrix))) * normal;

    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lPos -vec3(modelMatrix * vec4(position, 1.0)));
    vec3 col = clamp( amb * lParams.x + 
    lCol * max(0.0, dot(normal, lightDir)) + 
    specCol * pow(max(0.0, dot( normalize(camPos - vec3(modelMatrix * vec4(position, 1.0))), normalize( reflect(-lightDir, normal)))), lParams.y),
    0.0, 1.0);

    return col;
}

void main()
{       
        //  day
        // vec3 col = vec3(255.0/255.0, 226.0/255.0, 143.0/255.0) ; // light color
        // vec3 specCol = vec3(1.0);
        // vec3 ambCol = vec3(122.0/ 255.0, 157.0/255.0, 227.0/255.0);

        // //  peaceful night
        vec3 col = vec3(46.0/255.0 , 121.0/255.0, 196.0/255.0) * position.y * 1.5; // light color
        vec3 specCol = vec3(14.0/ 255.0, 200.0/255.0, 240.0/255.0);
        // vec3 specCol = vec3(255.0 / 255.0, 0.0 / 255.0, 130.0 / 255.0);
        vec3 ambCol = vec3(200.0/ 255.0, 152.0/255.0, 250.0/255.0);
        // vec3 ambCol = vec3(255.0 / 255.0, 0.0 / 255.0, 130.0 / 255.0);

        //no modification
        // vec3 col = vec3(1.0); // light color
        // vec3 specCol = vec3(1.0);
        // vec3 ambCol = vec3(1.0);
        
        // vec3 col = triangleColor;

        float attenuation_factor;
        vec3 lightsSum;
        for(int i = 0; i<numSnow; i++){
            float constAtt = 1.0;

            // //many snow
            // float linearAtt = 600.0;
            // float expAtt = 2.0;
            //few snow
            float linearAtt = 4000.0;
            float expAtt = 5.0;

            // float snowDistance = distance(snow[i], position);
            float snowDistance;
            // if(snow[i].y < position.y){
            //     snowDistance = 0.1;
            // }
            // else{
                snowDistance = length(position - snow[i]);
            // }
            
            attenuation_factor = 1/ (constAtt + linearAtt * snowDistance + expAtt * exp2(snowDistance));
            // if(snow[i].y > position.y){
            // vec3 snowCol = vec3(100.0/ 255.0, 53.0/255.0, 255.0/255.0);
            vec3 snowCol = vec3(4.0, 0.5, 1.0);
            vec3 snowIntensity = vec3(305.0, 150.0, 0.0);
            if(snowDistance < 0.1){
                // lightsSum += bhong(snow[i], snowCol, snowCol, snowCol, snowIntensity) * attenuation_factor;
                lightsSum += snowCol * attenuation_factor;
            }
            
            // }else{
            //     lightsSum += vec3(0.0);
            // }
            
        }

        

        vec3 offset = terrainOffset[gl_InstanceID];

        pos = vec3(modelMatrix * vec4(vec3(position.x,clamp(texture(tex, UV).x, 0.1, 5.0), position.z) + offset, 1.0));
        // pos = vec3(modelMatrix * vec4(position.x, Noise3D(vec3(position.x + offset.x,position.y, position.z + offset.z), 0.5), position.z , 1.0));
        
        color = bhong(lightPos, ambCol, col, specCol, lightParams) + lightsSum *30.0;

        // gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position + offset, 1.0);
        gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vec3(position.x,  texture(tex, UV).x, position.z) + offset, 1.0);
        // gl_Position =  vec4(vec3(position.x,  texture(tex, UV).x, position.z) + offset, 1.0);
        // gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position.x + offset.x, Noise3D(vec3(position.x + offset.x,position.y, position.z + offset.z), 0.5), position.z + offset.z , 1.0);
        norms = normal;

        // glpos =  projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

        texCoord = UV;
        // heightMap = HM;
        quv = QUV;
        qvert = qVert;
}
