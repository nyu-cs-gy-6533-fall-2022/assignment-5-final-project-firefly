// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// OpenGL Mathematics Library
#include <glm/glm.hpp>                  // glm::vec3
#include <glm/vec3.hpp>                 // glm::vec3
#include <glm/vec4.hpp>                 // glm::vec4
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

float rand3D(vec3 co)
{
    return fract(sin(dot(vec3(co.x, co.y, co.z), vec3(12.9898, 78.233, 144.7272))) * 43758.5453);
}

float simple_interpolate(float a, float b, float x)
{
    return a + smoothstep(0.f, 1.f, x) * (b - a);
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
    float v2 = rand3D(vec3(integer_x + 1.0, integer_y, integer_z));
    float v3 = rand3D(vec3(integer_x, integer_y + 1.0, integer_z));
    float v4 = rand3D(vec3(integer_x + 1.0, integer_y + 1.0, integer_z));

    float v5 = rand3D(vec3(integer_x, integer_y, integer_z + 1.0));
    float v6 = rand3D(vec3(integer_x + 1.0, integer_y, integer_z + 1.0));
    float v7 = rand3D(vec3(integer_x, integer_y + 1.0, integer_z + 1.0));
    float v8 = rand3D(vec3(integer_x + 1.0, integer_y + 1.0, integer_z + 1.0));

    float i1 = simple_interpolate(v1, v5, fractional_z);
    float i2 = simple_interpolate(v2, v6, fractional_z);
    float i3 = simple_interpolate(v3, v7, fractional_z);
    float i4 = simple_interpolate(v4, v8, fractional_z);

    float ii1 = simple_interpolate(i1, i2, fractional_x);
    float ii2 = simple_interpolate(i3, i4, fractional_x);

    return simple_interpolate(ii1, ii2, fractional_y);
}

float perlinNoise(vec3 coord, float wavelength)
{
    return interpolatedNoise3D(coord.x / wavelength, coord.y / wavelength, coord.z / wavelength);
}
