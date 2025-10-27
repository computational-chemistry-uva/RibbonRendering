#pragma once

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "input.h"
#include "spline.h"

// NOTE This currently does not handle deletion of GL resources, they are kept alive until termination of the program.

struct MeshVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct SphereVertex {
    glm::vec3 position;
};

struct CylinderVertex {
    glm::vec3 aPos;
    glm::vec3 bPos;
    glm::vec3 aCPN;
    glm::vec3 bCPN;
    glm::vec3 startDir;
};

// Simple struct to help with drawing
struct DrawObject {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    virtual void draw() = 0;
};

struct Mesh : DrawObject {
    GLuint ibo;
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;

    // TODO References?
    Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices);
    void draw();
};

struct Spheres : DrawObject {
    std::vector<SphereVertex> vertices;

    Spheres(std::vector<SphereVertex> vertices);
    void draw();
};

struct Cylinders : DrawObject {
    std::vector<CylinderVertex> vertices;

    Cylinders(std::vector<CylinderVertex> vertices);
    void draw();
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
Mesh createTubeMesh(BSpline& spline, int samples, int segments, float radius);
Spheres createSpheres(std::vector<glm::vec3> &points);
Cylinders createCylinders(std::vector<glm::vec3> &points);

// Draw DrawObject with given shader and uniform values
void draw(DrawObject &object, GLuint shaderProgram, Uniforms &uniforms);
