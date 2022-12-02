#version 330

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform int task;

in vec3 position;
in vec3 normal;
in vec2 UV;
in vec2 QUV;
in vec2 qVert;

out vec3 n;
flat out vec3 color;
// out vec3 color;
out vec3 pos;
out vec2 texCoord;
out vec3 norms;
out vec2 quv;
out vec2 qvert;
out vec4 glpos;

void main()
{       
        //low poly coloring

        n = mat3(transpose(inverse(modelMatrix))) * normal;

        vec3 col = triangleColor;
        vec3 normal = normalize(n);
        vec3 lightDir = normalize(lightPos -vec3(modelMatrix * vec4(position, 1.0)));
        col = clamp( col * lightParams.x + 
        col * max(0.0, dot(normal, lightDir)) + 
        vec3(1.0) * pow(max(0.0, dot( normalize(camPos - vec3(modelMatrix * vec4(position, 1.0))), normalize( reflect(-lightDir, normal)))), lightParams.y),
        0.0, 1.0);

        color = col;
        pos = vec3(modelMatrix * vec4(position, 1.0));
        gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
        norms = normal;

        // glpos =  projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

        texCoord = UV;

        quv = QUV;
        qvert = qVert;
}
