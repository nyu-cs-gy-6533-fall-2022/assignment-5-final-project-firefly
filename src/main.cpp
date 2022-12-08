// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include "Camera.h"
#include "noise.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp>                  // glm::vec3
#include <glm/vec3.hpp>                 // glm::vec3
#include <glm/vec4.hpp>                 // glm::vec4
#include <glm/mat4x4.hpp>               // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

// VertexBufferObject wrapper
BufferObject VBO_terrain;
// VertexBufferObject wrapper
BufferObject NBO_terrain;
// VertexBufferObject wrapper
BufferObject UVBO_terrain;
// VertexBufferObject wrapper
BufferObject IndexBuffer_terrain;

// VertexBufferObject wrapper
BufferObject VBO_sphere;
// VertexBufferObject wrapper
BufferObject NBO_sphere;
// VertexBufferObject wrapper
BufferObject UVBO_sphere;
// VertexBufferObject wrapper
BufferObject IndexBuffer_sphere;

// height map buffer object
BufferObject HMBO;

BufferObject QVBO;
BufferObject QUVBO;

// Contains the vertex positions
std::vector<glm::vec3> V(0);
// Contains the vertex positions
std::vector<glm::vec3> VN(0);
// Contains the vertex positions
std::vector<glm::vec2> UV(0);
// Contains the vertex indices
std::vector<glm::ivec3> T_terrain(0);
std::vector<glm::ivec3> T_sphere(0);
std::vector<glm::ivec3> T_model(0);
std::vector<glm::vec2> Q(6);
std::vector<glm::vec2> QUV(6);
std::vector<glm::vec3> HM(0);

// terrain texture
// std::vector<glm::vec3> pixels(0);
std::vector<GLfloat> pixels(0);

// Last position of the mouse on click
double xpos, ypos;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float mouse_sensitivity = 0.05f;

// camera setup and matrix calculations
Camera cam;

glm::mat4 viewMatrix;
glm::mat4 projMatrix;

// float camRadius = 5.0f;
// int task = 4;
int width, height;

// smooth camera
float deltaTime = 0.f;
float lastFrame = 0.f;

float movementSpd = 4.f;

// PPM Reader code from http://josiahmanson.com/prose/optimize_ppm/

struct RGB
{
    float r, g, b;
};

struct ImageRGB
{
    int w, h;
    std::vector<RGB> data;
};

void eat_comment(std::ifstream &f)
{
    char linebuf[1024];
    char ppp;
    while (ppp = f.peek(), ppp == '\n' || ppp == '\r')
        f.get();
    if (ppp == '#')
        f.getline(linebuf, 1023);
}

bool loadPPM(ImageRGB &img, const std::string &name)
{
    std::ifstream f(name.c_str(), std::ios::binary);
    if (f.fail())
    {
        std::cout << "Could not open file: " << name << std::endl;
        return false;
    }

    // get type of file
    eat_comment(f);
    int mode = 0;
    std::string s;
    f >> s;
    if (s == "P3")
        mode = 3;
    else if (s == "P6")
        mode = 6;

    // get w
    eat_comment(f);
    f >> img.w;

    // get h
    eat_comment(f);
    f >> img.h;

    // get bits
    eat_comment(f);
    int bits = 0;
    f >> bits;

    // error checking
    if (mode != 3 && mode != 6)
    {
        std::cout << "Unsupported magic number" << std::endl;
        f.close();
        return false;
    }
    if (img.w < 1)
    {
        std::cout << "Unsupported width: " << img.w << std::endl;
        f.close();
        return false;
    }
    if (img.h < 1)
    {
        std::cout << "Unsupported height: " << img.h << std::endl;
        f.close();
        return false;
    }
    if (bits < 1 || bits > 255)
    {
        std::cout << "Unsupported number of bits: " << bits << std::endl;
        f.close();
        return false;
    }

    // load image data
    img.data.resize(img.w * img.h);

    if (mode == 6)
    {
        f.get();
        f.read((char *)&img.data[0], img.data.size() * 3);
    }
    else if (mode == 3)
    {
        for (int i = 0; i < img.data.size(); i++)
        {
            int v;
            f >> v;
            img.data[i].r = v;
            f >> v;
            img.data[i].g = v;
            f >> v;
            img.data[i].b = v;
        }
    }

    // close file
    f.close();
    return true;
}

bool loadOFFFile(std::string filename, std::vector<glm::vec3> &vertex, std::vector<glm::ivec3> &tria, glm::vec3 &min, glm::vec3 &max)
{
    min.x = FLT_MAX;
    max.x = FLT_MIN;
    min.y = FLT_MAX;
    max.y = FLT_MIN;
    min.z = FLT_MAX;
    max.z = FLT_MIN;
    try
    {
        std::ifstream ofs(filename, std::ios::in | std::ios_base::binary);
        if (ofs.fail())
            return false;
        std::string line, tmpStr;
        // First line(optional) : the letters OFF to mark the file type.
        // Second line : the number of vertices, number of faces, and number of edges, in order (the latter can be ignored by writing 0 instead).
        int numVert = 0;
        int numFace = 0;
        int numEdge = 0;
        // first line must be OFF
        getline(ofs, line);
        if (line.rfind("OFF", 0) == 0)
            getline(ofs, line);
        std::stringstream tmpStream(line);
        getline(tmpStream, tmpStr, ' ');
        numVert = std::stoi(tmpStr);
        getline(tmpStream, tmpStr, ' ');
        numFace = std::stoi(tmpStr);
        getline(tmpStream, tmpStr, ' ');
        numEdge = std::stoi(tmpStr);

        // read all vertices and get min/max values
        V.resize(numVert);
        for (int i = 0; i < numVert; i++)
        {
            getline(ofs, line);
            tmpStream.clear();
            tmpStream.str(line);
            getline(tmpStream, tmpStr, ' ');
            V[i].x = std::stof(tmpStr);
            min.x = std::fminf(V[i].x, min.x);
            max.x = std::fmaxf(V[i].x, max.x);
            getline(tmpStream, tmpStr, ' ');
            V[i].y = std::stof(tmpStr);
            min.y = std::fminf(V[i].y, min.y);
            max.y = std::fmaxf(V[i].y, max.y);
            getline(tmpStream, tmpStr, ' ');
            V[i].z = std::stof(tmpStr);
            min.z = std::fminf(V[i].z, min.z);
            max.z = std::fmaxf(V[i].z, max.z);
        }

        // read all faces (triangles)
        T_model.resize(numFace);
        for (int i = 0; i < numFace; i++)
        {
            getline(ofs, line);
            tmpStream.clear();
            tmpStream.str(line);
            getline(tmpStream, tmpStr, ' ');
            if (std::stoi(tmpStr) != 3)
                return false;
            getline(tmpStream, tmpStr, ' ');
            T_model[i].x = std::stoi(tmpStr);
            getline(tmpStream, tmpStr, ' ');
            T_model[i].y = std::stoi(tmpStr);
            getline(tmpStream, tmpStr, ' ');
            T_model[i].z = std::stoi(tmpStr);
        }

        ofs.close();
    }
    catch (const std::exception &e)
    {
        // return false if an exception occurred
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

void sphere(float sphereRadius, int sectorCount, int stackCount, std::vector<glm::vec3> &vertex, std::vector<glm::vec3> &normal, std::vector<glm::ivec3> &tria)
{
    // init variables
    vertex.resize(0);
    normal.resize(0);
    T_sphere.resize(0);
    // temp variables
    glm::vec3 sphereVertexPos;
    float xy;
    float sectorStep = 2.0f * glm::pi<float>() / float(sectorCount);
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    // UV.resize(stackCount * sectorCount);

    std::vector<glm::vec2> tempVec;

    // compute vertices and normals
    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = glm::pi<float>() / 2.0f - i * stackStep;
        xy = sphereRadius * cosf(stackAngle);
        sphereVertexPos.z = sphereRadius * sinf(stackAngle);

        // glm::vec2 firstUV;
        for (int j = 0; j <= sectorCount; j++)
        {

            sectorAngle = j * sectorStep;

            // vertex position
            sphereVertexPos.x = xy * cosf(sectorAngle);
            sphereVertexPos.y = xy * sinf(sectorAngle);
            vertex.push_back(sphereVertexPos);

            normal.push_back(sphereVertexPos / sphereRadius);

            // pushback Texcoords
            UV.push_back(glm::vec2(((float)j) / ((float)sectorCount), ((float)i) / (float)stackCount));
        }
    }

    // compute triangle indices
    int k1, k2;

    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                T_sphere.push_back(glm::ivec3(k1, k2, k1 + 1));
            }
            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                T_sphere.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
            }
        }
    }
}
void createFireflies(int &nFlies, std::vector<glm::vec3> &translations, std::vector<glm::vec3> &vectors)
{
    translations.resize(0);
    for (int i = 0; i < nFlies; i++)
    {
        translations.push_back(glm::vec3(((float)rand() / (float)RAND_MAX), (float)rand() / (float)RAND_MAX * 8.f, ((float)rand() / (float)RAND_MAX)));
        vectors.push_back(glm::vec3(0.f));
    }
}

void createTerrainInstances(int &nTerrain, std::vector<glm::vec3> &translations)
{
    translations.resize(0);
    translations.push_back(glm::vec3(0.f));
    for (int i = 0; i < nTerrain; i++)
    {                   // construct instanced terrain in counter-clockwise order
        if (i % 9 == 0) // back middle
        {
            translations.push_back(glm::vec3(0.f, 0.f, (float)(1.f + i / 9)));
        }
        else if (i % 9 == 1) // back left
        {
            translations.push_back(glm::vec3((float)(1.f + i / 9), 0.f, (float)(1.f + i / 9)));
        }
        else if (i % 9 == 2) // middle left
        {
            translations.push_back(glm::vec3((float)(1.f + i / 9), 0.f, 0.f));
        }
        else if (i % 9 == 3) // front left
        {
            translations.push_back(glm::vec3((float)(1.f + i / 9), 0.f, (float)-(1.f + i / 9)));
        }
        else if (i % 9 == 4) // front middle
        {
            translations.push_back(glm::vec3(0.f, 0.f, (float)-(1.f + i / 9)));
        }
        else if (i % 9 == 5) // front right
        {
            translations.push_back(glm::vec3((float)-(1.f + i / 9), 0.f, (float)-(1.f + i / 9)));
        }
        else if (i % 9 == 6) // middle right
        {
            translations.push_back(glm::vec3((float)-(1.f + i / 9), 0.f, 0.f));
        }
        else if (i % 9 == 7) // back right
        {
            translations.push_back(glm::vec3((float)-(1.f + i / 9), 0.f, (float)(1.f + i / 9)));
        }
    }
}

glm::vec3 biome(float elevation)
{
    if (elevation < 0.1)
    { // WATER BIOME
        return vec3(68.0 / 255.0, 68.0 / 255.0, 120.0 / 255.0);
    }
    else if (elevation < 0.2)
    { // BEACH
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
    else if (elevation < 0.3)
    { // FOREST
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
    else if (elevation < 0.4)
    { // DEEP FOREST
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
    else if (elevation < 0.6)
    { // STEPPES
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
    else if (elevation < 0.8)
    { // STONE
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
    else if (elevation < 0.9)
    { // SNOW
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
    else
    { // SNOW CAP
        return vec3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
    }
}

void terrain(float freq, int cols, int rows, int width, int height, std::vector<glm::vec3> &vertex, std::vector<glm::vec3> &normal, std::vector<RGB> &hm)
{
    vertex.resize(0);
    normal.resize(0);
    T_terrain.resize(0);
    hm.resize((rows + 1) * (cols + 1));

    // create terrain plane vertices
    float colSec = width / cols;
    float rowSec = height / rows;
    for (int r = 0; r <= rows; r++)
    {
        for (int c = 0; c <= cols; c++)
        {
            double nx = (double)r * rowSec / height - 0.5;
            double ny = (double)c * colSec / width - 0.5;
            // float perlinSum = -0.0f;

            float perlinSum = (glm::perlin(glm::vec2(nx * freq, ny * freq)) + glm::perlin(glm::vec2(nx * freq * 3.f, ny * freq / 3.f)) / 2.f);

            vertex.push_back(glm::vec3((float)c * colSec / (float)width, 0.f, (float)r * rowSec / (float)height));

            hm[r * (cols + 1) + c].r = ((perlinSum + 1.f) / 2.f);
            hm[r * (cols + 1) + c].g = ((perlinSum + 1.f) / 2.f);
            hm[r * (cols + 1) + c].b = ((perlinSum + 1.f) / 2.f);

            // hm[r * (cols + 1) + c].r = 52;
            // hm[r * (cols + 1) + c].g = 52;
            // hm[r * (cols + 1) + c].b = 108;
            // std::cout << r * (cols + 1) + c << std::endl;
            // std::cout << "H:" << (float)hm[r * (cols + 1) + c].r << std::endl;

            UV.push_back(glm::vec2((float)c / (float)(cols), (float)r / (float)(rows)));

            // std::cout << (float)c / (float)(cols) << " : " << (float)r / (float)(rows) << std::endl;
            // normal.push_back
            // UV.push_back(glm::vec2((float)c / (float)cols, (float)r / (float)rows));
            // std::cout << vertex[vertex.size() - 1].x << std::endl;
            // std::cout << perlinSum + 0.5f << std::endl;
            // pixels.push_back(glm::vec3(0.3f, 0.2f, 0.66f));
        }
    }
    std::cout << vertex.size() << std::endl;
    // vertex[222] += glm::vec3(0.f, 0.1f, 0.f);

    // for (int i = 0; i < rows; i++)
    // {
    //     for (int j = 0; j < cols; j++)
    //     {
    //         double nx = (double)i / vertex.size() - 0.5;
    //         double ny = (double)j / vertex.size() - 0.5;

    //         // float perlinSum = (glm::perlin(glm::vec2(nx * freq, ny * freq)));
    //         float perlinSum = perlinNoise(glm::vec3(nx, 0.f, ny), freq);

    //         UV.push_back(glm::vec2((float)i / (float)vertex.size(), (float)j / (float)vertex.size()));

    //         // std::cout << i << " : " << j << std::endl;
    //         // std::cout << ((perlinSum + 1.f) / 2.f) * 255.f << std::endl;

    //         hm[vertex.size() * i + j].r = ((perlinSum + 1.f) / 2.f) * 255.f;
    //         hm[vertex.size() * i + j].g = ((perlinSum + 1.f) / 2.f) * 255.f;
    //         hm[vertex.size() * i + j].b = ((perlinSum + 1.f) / 2.f) * 255.f;

    //         // hm[vertex.size() * i + j].r = 52.f;
    //         // hm[vertex.size() * i + j].g = 52.f;
    //         // hm[vertex.size() * i + j].b = 108.f;
    //     }
    // }

    // create terrain plane triangles index buffer
    int k1, k2;

    for (int i = 0; i < rows; ++i)
    {
        k1 = i * (cols + 1);
        k2 = k1 + cols + 1;

        for (int j = 0; j < cols; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1

            // T.push_back(glm::ivec3(k1, k2, k1 + 1));

            // T.push_back(glm::ivec3(k2 + 1, k1 + 1, k2));
            // if (i % 2 == 0)
            // {

            if (j % 2 == 0)
            {

                T_terrain.push_back(glm::ivec3(k1, k2, k2 + 1));

                T_terrain.push_back(glm::ivec3(k2 + 1, k1 + 1, k1));
            }
            else
            {
                T_terrain.push_back(glm::ivec3(k2, k1 + 1, k1));

                T_terrain.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
            }
            // }
            // else
            // {
            //     if (j % 2 == 0)
            //     {
            //         T.push_back(glm::ivec3(k2, k1 + 1, k1));

            //         T.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
            //     }
            //     else
            //     {
            //         T.push_back(glm::ivec3(k1, k2, k2 + 1));

            //         T.push_back(glm::ivec3(k2 + 1, k1 + 1, k1));
            //     }
            // }
        }
    }

    normal.resize(T_terrain.size());
    // calculate surface normals
    for (int i = 0; i < T_terrain.size(); i++)
    {
        glm::vec3 u = vertex[T_terrain[i].y] - vertex[T_terrain[i].x];
        glm::vec3 v = vertex[T_terrain[i].z] - vertex[T_terrain[i].x];

        normal[T_terrain[i].x] += glm::vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
        normal[T_terrain[i].y] += glm::vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
        normal[T_terrain[i].z] += glm::vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);

        // normal[T[i].x] += glm::normalize(glm::cross(V[T[i].y] - V[T[i].x], V[T[i].z] - V[T[i].x]));

        // normal[T[i].y] += glm::normalize(glm::cross(V[T[i].y] - V[T[i].x], V[T[i].z] - V[T[i].x]));

        // normal[T[i].z] += glm::normalize(glm::cross(V[T[i].y] - V[T[i].x], V[T[i].z] - V[T[i].x]));
    }

    for (int i = 0; i < normal.size(); i++)
    {
        normal[i] = glm::normalize(normal[i]);
    }

    // procedural terrain noise
    // for (int y = 0; y < height; y++)
    // {
    //     for (int x = 0; x < width; x++)
    //     {
    //         double nx = (double)x / width - 0.5;
    //         double ny = (double)y / height - 0.5;

    //         HM.push_back(glm::vec3(glm::perlin(glm::vec2(nx, ny))));
    //         // std::cout << HM[HM.size() - 1].x << std::endl;
    //         // HM[HM.size() - 1] = glm::normalize(HM[HM.size() - 1]);
    //     }
    // }

    // GLuint noiseTex;
    // glGenTextures(1, &noiseTex);

    // glBindTexture(GL_TEXTURE_2D, noiseTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, HM.data());

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // delete (&HM);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Get the position of the mouse in the window
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << xpos << " " << ypos << std::endl;
    }
}

void mouse_movement_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;

        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    cam.yaw += xoffset * mouse_sensitivity;
    cam.pitch += yoffset * mouse_sensitivity;

    cam.pitch = glm::clamp(cam.pitch, -89.f, 89.f);

    glm::vec3 dir;
    dir.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
    dir.y = sin(glm::radians(cam.pitch));
    dir.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
    // std::cout << xpos;
    // std::cout << xoffset << std::endl;
    cam.cameraFront = glm::normalize(dir);
    cam.cameraRight = glm::normalize(glm::cross(cam.upValue, cam.cameraFront));
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // temp variables
    glm::mat3 rot;
    switch (key)
    {
    case GLFW_KEY_A:
        cam.cameraPos += cam.cameraRight * movementSpd * deltaTime;
        break;
    case GLFW_KEY_D:
        cam.cameraPos -= cam.cameraRight * movementSpd * deltaTime;
        break;
    case GLFW_KEY_W:
        cam.cameraPos += cam.cameraFront * movementSpd * deltaTime;
        break;
    case GLFW_KEY_S:
        cam.cameraPos -= cam.cameraFront * movementSpd * deltaTime;
        break;
    case GLFW_KEY_SPACE:
        cam.cameraPos += glm::vec3(0.0, movementSpd * deltaTime, 0.0f);
        break;
    case GLFW_KEY_LEFT_CONTROL:
        cam.cameraPos += glm::vec3(0.0, -movementSpd * deltaTime, 0.0f);
        break;
    case GLFW_KEY_UP:
        cam.cameraPos -= cam.cameraDir * movementSpd * deltaTime;
        break;
    case GLFW_KEY_DOWN:
        cam.cameraPos += cam.cameraDir * movementSpd * deltaTime;
        break;
    case GLFW_KEY_R:
        // cam.cameraPos = glm::vec3(0.0f, 0.0f, camRadius);
        cam.cameraPos = glm::vec3(0.5f, 0.f, 0.f);
        cam.cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        cam.cameraDir = glm::normalize(cam.cameraPos - cam.cameraTarget);
        cam.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cam.cameraFront = glm::vec3(0.0f, 0.f, 1.f);
        cam.cameraRight = glm::normalize(glm::cross(cam.upValue, cam.cameraFront));
        cam.yaw = 90.f;
        cam.pitch = 0.f;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    default:
        break;
    }
}

int main(void)
{
    GLFWwindow *window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1600, 900, "Firefly", NULL, NULL);

    // take focus of cursor
    glfwSetCursorPos(window, 400, 300);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // get cursorpos callback
    glfwSetCursorPosCallback(window, mouse_movement_callback);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
    glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
#endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    std::cout << "OpenGL version recieved: " << major << "." << minor << "." << rev << std::endl;
    std::cout << "Supported OpenGL is " << (const char *)glGetString(GL_VERSION) << std::endl;
    std::cout << "Supported GLSL is " << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    std::stringstream screenfragCode;
    std::stringstream screenvertCode;
    std::stringstream fragCode;
    std::stringstream vertCode;
    Program program_firefly;
    std::stringstream fragCode_fireFly;
    std::stringstream vertCode_fireFly;

    std::ifstream fragShaderFile("../shader/fragment.glsl");

    fragCode << fragShaderFile.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile("../shader/vertex.glsl");

    vertCode << vertShaderFile.rdbuf();
    // load fragment shader file
    std::ifstream fragShaderFile2("../shader/rendtexFragment.glsl");

    screenfragCode << fragShaderFile2.rdbuf();
    // // load vertex shader file
    std::ifstream vertShaderFile2("../shader/rendtexVertex.glsl");

    screenvertCode << vertShaderFile2.rdbuf();

    // load fragment shader file
    std::ifstream fragShaderFile3("../shader/fragment_firefly.glsl");

    fragCode_fireFly << fragShaderFile3.rdbuf();
    // // load vertex shader file
    std::ifstream vertShaderFile3("../shader/vertex_firefly.glsl");

    vertCode_fireFly << vertShaderFile3.rdbuf();
    program_firefly.init(vertCode_fireFly.str(), fragCode_fireFly.str(), "outColor");

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertCode.str(), fragCode.str(), "outColor");
    program.bind();

    // program.bindVertexAttribArray("HM", HMBO);
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // // Update viewport
    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize the VAO_terrain
    // A Vertex Array Object (or VAO_terrain) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO_terrain is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO_terrain;
    VertexArrayObject VAO_sphere;
    VertexArrayObject QAO;
    VAO_terrain.init();

    VAO_terrain.bind();

    // Initialize the VBO with the vertices data
    VBO_terrain.init();
    // initialize normal array buffer
    NBO_terrain.init();
    // intialize uv coord array buffer
    UVBO_terrain.init();
    // height map buffer
    std::vector<RGB> HM;
    // initialize element array buffer
    IndexBuffer_terrain.init(GL_ELEMENT_ARRAY_BUFFER);

    // sphere(1.5f, 10, 20, V, VN, T);
    int terrainCols = 25;
    int terrainRows = 25;
    int terrainWidth = 500;
    int terrainHeight = 500;
    terrain(2.0f, terrainCols, terrainRows, terrainWidth, terrainHeight, V, VN, HM);

    // std::cout << V[10].x << std::endl;
    VBO_terrain.update(V);
    NBO_terrain.update(VN);
    UVBO_terrain.update(UV);
    IndexBuffer_terrain.update(T_terrain);
    // HMBO.update(HM);

    // bind vertex buffer objects to the shader
    program.bindVertexAttribArray("position", VBO_terrain);
    program.bindVertexAttribArray("normal", NBO_terrain);
    program.bindVertexAttribArray("UV", UVBO_terrain);

    int numTerrain = 1; // total - 1 terrain grids
    std::vector<glm::vec3> translations_terrain(numTerrain);
    createTerrainInstances(numTerrain, translations_terrain);

    VAO_sphere.init();
    VAO_sphere.bind();

    // Initialize the VBO with the vertices data
    VBO_sphere.init();
    // initialize normal array buffer
    NBO_sphere.init();
    // intialize uv coord array buffer
    UVBO_sphere.init();
    // height map buffer
    // HMBO.init();
    // initialize element array buffer
    IndexBuffer_sphere.init(GL_ELEMENT_ARRAY_BUFFER);

    // bind to firefly shader
    program_firefly.bind();
    // generate sphere (radius, #sectors, #stacks, vertices, normals, triangle indices)
    sphere(0.004f, 5, 5, V, VN, T_sphere);

    VBO_sphere.update(V);
    NBO_sphere.update(VN);
    UVBO_sphere.update(UV);
    IndexBuffer_sphere.update(T_sphere);

    // bind vertex buffer objects to the shader
    program_firefly.bindVertexAttribArray("position", VBO_sphere);
    program_firefly.bindVertexAttribArray("normal", NBO_sphere);
    program_firefly.bindVertexAttribArray("UV", UVBO_sphere);

    // create biome texture
    GLuint biomeTex;
    glGenTextures(1, &biomeTex);

    glBindTexture(GL_TEXTURE_2D, biomeTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::cout << (terrainCols + 1) * (terrainRows + 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, terrainCols + 1, terrainRows + 1, 0, GL_RGB, GL_FLOAT, &HM[0]); // make array of unsigned char instead of vec3

    // random instance translations of fireflies
    srand(time(NULL));
    int numFlies = 100;
    std::vector<glm::vec3> translations_spheres(numFlies);
    std::vector<glm::vec3> spheres_vectors(numFlies);

    createFireflies(numFlies, translations_spheres, spheres_vectors);

    Program program_rendTex;
    program_rendTex.init(screenvertCode.str(), screenfragCode.str(), "outColor");
    program_rendTex.bind();
    // glUniform1i(program.uniform("task"), task);

    // setup quad for render to texture
    glBindVertexArray(0);
    QAO.init();
    Q = {
        // bottom left triangle
        glm::vec2(1.f, -1.f),
        glm::vec2(-1.f, -1.f),
        glm::vec2(-1.f, 1.f),
        // top right triangle
        glm::vec2(1.f, 1.f),
        glm::vec2(1.f, -1.f),
        glm::vec2(-1.f, 1.f)};

    QUV = {
        glm::vec2(1.f, 0.f),
        glm::vec2(0.f, 0.f),
        glm::vec2(0.f, 1.f),
        glm::vec2(1.f, 1.f),
        glm::vec2(1.f, 0.f),
        glm::vec2(0.f, 1.f),
    };

    QVBO.init();
    QAO.bind();

    QVBO.bind();
    QVBO.update(Q);

    program_rendTex.bindVertexAttribArray("qVert", QVBO);

    QUVBO.init();
    QUVBO.bind();

    QUVBO.update(QUV);
    program_rendTex.bindVertexAttribArray("QUV", QUVBO);

    glBindVertexArray(0);

    GLuint FBO;
    glfwGetWindowSize(window, &width, &height);

    glUniform1i(program_rendTex.uniform("width"), width);
    glUniform1i(program_rendTex.uniform("height"), height);

    // setup framebuffer for render to texture
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    GLuint TBO_color;
    GLuint TBO_depth;
    GLuint TBO_norm;
    GLuint TBO_pos;

    // color texture
    glGenTextures(1, &TBO_color);
    glBindTexture(GL_TEXTURE_2D, TBO_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TBO_color, 0);

    // normal texture
    glGenTextures(1, &TBO_norm);
    glBindTexture(GL_TEXTURE_2D, TBO_norm);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TBO_norm, 0);

    // position texture
    glGenTextures(1, &TBO_pos);
    glBindTexture(GL_TEXTURE_2D, TBO_pos);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, TBO_pos, 0);

    // depth texture
    glGenTextures(1, &TBO_depth);
    glBindTexture(GL_TEXTURE_2D, TBO_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TBO_depth, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TBO_depth, 0);

    GLenum DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, DrawBuffers); // "1" is the size of DrawBuffers

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
        return false;
    }

    // Set the list of draw buffers.
    // 0: color, 1: norm, 2: position

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    program.init(vertCode.str(), fragCode.str(), "outColor");

    // initialize model matrix
    glm::mat4 modelMatrix_terrain = glm::mat4(1.0f);
    glm::mat4 modelMatrix_sphere = glm::mat4(1.0f);
    // modelMatrix = glm::rotate(modelMatrix, glm::radians(90.f), glm::vec3(1, 0, 0));
    // modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.f), glm::vec3(0, 1, 0));

    int tempFlies = numFlies;
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {

        numFlies = tempFlies;
        // smooth camera operations
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // std::cout << deltaTime << std::endl;

        // matrix calculations
        viewMatrix = glm::lookAt(cam.cameraPos, cam.cameraPos + cam.cameraFront, cam.cameraUp);
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);

        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindTexture(GL_TEXTURE_2D, biomeTex);

        // clear framebuffer
        // glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClearColor(22.f / 255.f, 22.f / 255.f, 47.f / 255.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);

        // SPHERES MUST BE RENDERED FIRST TO ENSURE THAT OFFSET POSITION IS CORRECT

        program_firefly.bind();
        glUniform3f(program_firefly.uniform("triangleColor"), 1.0f, 0.5f, 0.0f);
        glUniform3f(program_firefly.uniform("camPos"), cam.cameraPos.x, cam.cameraPos.y, cam.cameraPos.z);

        glUniformMatrix4fv(program_firefly.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(program_firefly.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        // direction towards the light
        glUniform3fv(program_firefly.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(-1.0f, 2.0f, -3.0f)));
        // glUniform3fv(program_firefly.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(-1.0f + currentFrame * 5.f, 2.0f, -3.0f)));
        // x: ambient;
        glUniform3f(program_firefly.uniform("lightParams"), 0.35f, 50.0f, 0.0f);

        { // bind and draw sphere VAO
            VAO_sphere.bind();

            // use terrain render to texture to render fireflies above terrain
            // glActiveTexture(GL_TEXTURE2);
            // glBindTexture(GL_TEXTURE_2D, TBO_pos);

            // glUniform1i(program_firefly.uniform("texPos"), 2);
            // glActiveTexture(GL_TEXTURE0);
            // glUniform1i(program_firefly.uniform("numFlies"), randInt);
            float randFloat = (2 * ((float)rand() / (float)RAND_MAX) - 1.f) / 100.f;

            float closestSnow = 999999999.f;
            for (unsigned int i = 0; i < numFlies; i++)
            {

                float speed_reduction = 200000.f;
                float fall_speed = -0.003f;
                // float altSign = (cosf(currentFrame) > 0) - (cosf(currentFrame) < 0);

                if (translations_spheres[(int)i].y >= -1.f)
                {
                    spheres_vectors[(int)i] = glm::vec3(spheres_vectors[(int)i].x + (((float)rand() / (float)RAND_MAX) - 0.5f) / speed_reduction, spheres_vectors[(int)i].y + (((float)rand() / (float)RAND_MAX) - 0.5f) / (speed_reduction), spheres_vectors[(int)i].z + (((float)rand() / (float)RAND_MAX) - 0.5f) / speed_reduction);
                    translations_spheres[(int)i] = translations_spheres[(int)i] + spheres_vectors[(int)i] + glm::vec3(0.f, fall_speed, 0.f);
                }
                else
                {
                    translations_spheres[(int)i] = glm::vec3(((float)rand() / (float)RAND_MAX), 8.f, ((float)rand() / (float)RAND_MAX));
                    spheres_vectors[(int)i] = glm::vec3(0.f);
                }

                // std::cout << glm::distance(translations_spheres[(int)i], cam.cameraPos) << std::endl;
                float distanceToSnow = glm::distance(cam.cameraPos, translations_spheres[(int)i]);
                if (distanceToSnow < closestSnow)
                {
                    closestSnow = distanceToSnow;
                }

                // std::cout << spheres_vectors[(int)i].x << std::endl;
                // std::cout << (2 * ((float)rand() / (float)RAND_MAX) - 1.f) << std::endl;
                glUniform3fv(program_firefly.uniform("offsets[" + std::to_string(i) + "]"), 1, glm::value_ptr(translations_spheres[(int)i]));
            }

            if (closestSnow > 4.f) // if distance from closest snow to camera is >4.f then stop rendering
            {
                numFlies = 0;
            }
            IndexBuffer_sphere.bind();
            // modelMatrix_sphere = glm::translate(glm::mat4(1.0f), glm::vec3(sinf(currentFrame) * 0.1f, 1.f, cosf(currentFrame) * 0.1f));
            modelMatrix_sphere = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 0.f));
            glUniformMatrix4fv(program_firefly.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix_sphere));
            // // Draw a triangle

            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElementsInstanced(GL_TRIANGLES, T_sphere.size() * 3, GL_UNSIGNED_INT, 0, numFlies);
        }

        // Bind your program terrain
        program.bind();

        // lighting setup
        //  Set the uniform values

        glUniform3f(program.uniform("triangleColor"), 1.0f, 0.5f, 0.0f);
        glUniform3f(program.uniform("camPos"), cam.cameraPos.x, cam.cameraPos.y, cam.cameraPos.z);

        glUniformMatrix4fv(program.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(program.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        // direction towards the light
        glUniform3fv(program.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(-1.0f, 2.0f, -3.0f)));
        // glUniform3fv(program.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(-1.0f + currentFrame * 5.f, 2.0f, -3.0f)));
        // x: ambient;
        glUniform3f(program.uniform("lightParams"), 0.45f, 50.0f, 0.0f);

        { // bind and draw terrain VAO
            VAO_terrain.bind();
            modelMatrix_terrain = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 0.f));
            glUniformMatrix4fv(program.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix_terrain));
            IndexBuffer_terrain.bind();

            // // Draw a triangle
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            for (int i = 0; i < numFlies; i++)
            {
                glUniform1i(program.uniform("numSnow"), numFlies);
                // temp vector to have light position be ahead of sphere position
                glm::vec3 translationTemp = translations_spheres[(int)i] + glm::vec3(0.f, 0.f, 0.f);
                glUniform3fv(program.uniform("snow[" + std::to_string(i) + "]"), 1, glm::value_ptr(translationTemp));
            }

            for (int i = 0; i < numTerrain; i++)
            {
                // glUniform1i(program.uniform("numTerrain"), numTerrain);

                glUniform3fv(program.uniform("terrainOffset[" + std::to_string(i) + "]"), 1, glm::value_ptr(translations_terrain[(int)i]));
            }
            glDrawElementsInstanced(GL_TRIANGLES, T_terrain.size() * 3, GL_UNSIGNED_INT, 0, numTerrain);
        }

        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Clear the framebuffer

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // // program.init(screenvertCode.str(), screenfragCode.str(), "outColor");
        program_rendTex.bind();

        glUniform3f(program_rendTex.uniform("camPos"), cam.cameraPos.x, cam.cameraPos.y, cam.cameraPos.z);
        glUniformMatrix4fv(program_rendTex.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniform3f(program_rendTex.uniform("triangleColor"), 1.0f, 0.5f, 0.0f);
        // direction towards the light
        glUniform3fv(program_rendTex.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(-1.0f, 2.0f, 3.0f)));
        // x: ambient;
        glUniform3f(program_rendTex.uniform("lightParams"), 0.1f, 50.0f, 0.0f);

        QAO.bind();

        glDisable(GL_DEPTH_TEST);
        // glEnable(GL_DEPTH_TEST);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TBO_color);
        glUniform1i(program_rendTex.uniform("texColor"), 0);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, TBO_color);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, TBO_norm);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, TBO_pos);
        // glActiveTexture(GL_TEXTURE3);
        // glBindTexture(GL_TEXTURE_2D, TBO_depth);

        // // connect sampler2D uniforms to which texture unit
        // glUniform1i(program_rendTex.uniform("texColor"), 0);
        // glUniform1i(program_rendTex.uniform("texNorm"), 1);
        // glUniform1i(program_rendTex.uniform("texPos"), 2);
        // glUniform1i(program_rendTex.uniform("texDepth"), 3);
        // glActiveTexture(GL_TEXTURE0);

        glDrawArrays(GL_TRIANGLES, 0, Q.size());

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    program_firefly.free();
    program_rendTex.free();
    VAO_terrain.free();
    VBO_terrain.free();
    UVBO_terrain.free();
    IndexBuffer_terrain.free();

    VAO_sphere.free();
    VBO_sphere.free();
    UVBO_sphere.free();
    IndexBuffer_sphere.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
