#version 330

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform int task;

in vec2 position;
in vec3 normal;
in vec2 UV;
in vec2 QUV;
in vec2 qVert;

out vec3 n;
out vec3 pos;
out vec3 color;
out vec2 texCoord;
out vec3 norms;

void main()
{
    color = triangleColor;

    gl_Position =  vec4(qVert, 0, 1.0);
    pos = vec3(qVert, 0);
    texCoord = QUV;

}
