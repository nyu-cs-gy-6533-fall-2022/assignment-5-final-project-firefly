#version 330

out vec4 outColor;

in vec2 texCoord;
in vec3 n;
in vec3 color;
in vec3 pos;
in vec3 norms;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
// uniform int task;

uniform int width;
uniform int height;

uniform sampler2D texColor;
uniform sampler2D texNorm;
uniform sampler2D texPos;
uniform sampler2D texDepth;

// uniform sampler2D tex[3];

void main()
{   

    vec3 position = texture(texPos, texCoord).rgb;
    vec3 normal = texture(texNorm, texCoord).rgb;
    vec3 color = texture(texColor, texCoord).rgb;

    // vec3 lightDir = normalize(lightPos - position);
    // vec3 col = clamp( color * lightParams.x + 
    //     color * max(0.0, dot(normal, lightDir)) + 
    //     vec3(1.0) * pow(max(0.0, dot( normalize(camPos - position), normalize( reflect(-lightDir, normal)))), lightParams.y),
    //     0.0, 1.0);

    // float pixelX = 1.0/width;
    // float pixelY = 1.0/height;

    // float topLeftCoord = texture(texDepth, texCoord + vec2(-pixelX, pixelY)).x;
    // float topRightCoord = texture(texDepth, texCoord + vec2(pixelX, pixelY)).x;
    // float bottomLeftCoord = texture(texDepth, texCoord + vec2(-pixelX, -pixelY)).x;
    // float bottomRightCoord = texture(texDepth, texCoord + vec2(pixelX,-pixelY)).x;

    // float edge = sqrt(pow((topRightCoord - bottomLeftCoord),2) + pow((topLeftCoord - bottomRightCoord),2));

    if(color == vec3(0.0, 0.0, 0.0)){
        outColor = vec4(0.5, 0.5, 0.5, 1.0);
    }else{
        outColor = vec4(color, 1.0);
    }
    // if( (abs(texture(texDepth, texCoord).x - topLeftCoord)>0.0002 || abs(texture(texDepth, texCoord).x - topRightCoord)>0.0002 || abs(texture(texDepth, texCoord).x - bottomLeftCoord)>0.0002 || abs(texture(texDepth, texCoord).x - bottomRightCoord)>0.0002 )){
    //     outColor = vec4(0.0, 0.0, 0.0, 1.0);
    // }
}
