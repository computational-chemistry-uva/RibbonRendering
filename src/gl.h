#pragma once

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "input.h"
#include "spline.h"

// NOTE This currently does not handle deletion of GL resources, they are kept alive until termination of the program.

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

// Simple struct to help with drawing
// TODO Maybe actually use inheritance to differentiate types of objects
struct DrawObject {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

struct Camera {
    float yaw = -25.0f;
    float pitch = 25.0f;
    float dist = 25.0f;
    float fov = 45.0f;

    // Update camera parameters based on mouse state
    void update(MouseState mouse);
};

// Uniform data to send to shaders
// TODO Some of these should be vertex attributes so that multiple instances
//      with different parameters can be drawn in one draw call
struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 lightPos;
    bool drawNormals = false;
    bool drawTexture = true;
    bool checkerboard = false;
    //float lightIntensity = 0.65f;
    float lightIntensity = 0.0f;
    //float ambientLightIntensity = 0.35f;
    float ambientLightIntensity = 1.0f;
    float sphereRadius = 0.25f;
    bool raytraced = true;
    float cylinderRadius = 0.1f;
    int cylinderMode = 0;
    float pitch = 0.5f;
    float width = 0.25f;

    // Update MVP matrices and light position
    void updateMatrices(GLFWwindow *window, Camera &camera);
};

// Load vertex and fragment shader from files and compile them into a shader program
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);

struct Shaders {
    GLuint meshWireframeProgram = createShaderProgram(
        "../src/shaders/mesh_vertex.glsl",
        "../src/shaders/wireframe_fragment.glsl"
    );
    GLuint meshProgram = createShaderProgram(
        "../src/shaders/mesh_vertex.glsl",
        "../src/shaders/mesh_fragment.glsl"
    );
    GLuint sphereWireframeProgram = createShaderProgram(
        "../src/shaders/sphere_vertex.glsl",
        "../src/shaders/wireframe_fragment.glsl"
    );
    GLuint sphereProgram = createShaderProgram(
        "../src/shaders/sphere_vertex.glsl",
        "../src/shaders/sphere_fragment.glsl"
    );
    GLuint cylinderWireframeProgram = createShaderProgram(
        "../src/shaders/cylinder_vertex.glsl",
        "../src/shaders/wireframe_fragment.glsl"
    );
    GLuint cylinderProgram = createShaderProgram(
        "../src/shaders/cylinder_vertex.glsl",
        "../src/shaders/cylinder_fragment.glsl"
    );
};

// Helper functions to create DrawObjects from a set of input points
DrawObject createMesh(std::vector<float> &vertices, std::vector<unsigned int> &indices);
DrawObject createTubeMesh(BSpline& spline, int samples, int segments, float radius);
DrawObject createSpheres(std::vector<glm::vec3> &points);
DrawObject createCylinders(std::vector<glm::vec3> &points);

// Draw DrawObject with given shader and uniform values
void draw(DrawObject &object, GLuint shaderProgram, Uniforms &uniforms);
