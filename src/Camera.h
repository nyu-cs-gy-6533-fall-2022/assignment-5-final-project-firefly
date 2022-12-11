#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

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

class Camera
{
public:
    // Camera setup
    vec3 cameraPos;
    vec3 cameraTarget;
    vec3 cameraFront;
    vec3 cameraDir;
    float distanceFromOrigin;

    vec3 cameraRight;
    vec3 cameraUp;

    // Contains the view transformation
    mat4 view;

    float yaw;
    float pitch;

    float fov;
    float nearValue;
    float farValue;
    vec3 upValue;

    Camera(vec3 position = vec3(1.29762f, 2.66361f, -0.317415f), vec3 up = vec3(0.f, 1.f, 0.f), vec3 target = vec3(0.f, 0.f, 0.f), vec3 front = vec3(-0.331012f, -0.886204f, 0.324151f), float fieldOfView = 35.f, float nearVal = 0.1f, float farVal = 100.f)
    {
        cameraPos = position;
        cameraTarget = target;
        cameraFront = front;
        cameraDir = normalize(position - target);
        cameraRight = normalize(cross(up, cameraFront));
        upValue = up;
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        view = lookAt(cameraPos, cameraTarget, up);
        fov = fieldOfView;
        nearValue = nearVal;
        farValue = farVal;
        yaw = 135.65f;
        pitch = -62.4f;
        distanceFromOrigin = distance(vec3(0.f, 0.f, 0.f), cameraPos);
    };

private:
};
#endif