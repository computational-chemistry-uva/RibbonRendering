#pragma once

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "input.h"
#include "spline.h"

struct Camera {
    float yaw = -25.0f;
    float pitch = 25.0f;
    float dist = 25.0f;
    float fov = 45.0f;

    // Update camera parameters based on mouse state
    void update(MouseState mouse);
};

// Load vertex and fragment shader from files and compile them into a shader program
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);

// TODO Destructor
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

// Uniform data to send to shaders
// TODO Some of these should be vertex attributes so that multiple instances
//      with different parameters can be drawn in one draw call
// TODO Mode to only draw texture
// TODO Apply ambientLightIntensity in sphere and cylinder shaders
struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 lightPos;
    bool drawNormals = false;
    int drawTexture = 2;
    bool checkerboard = false;
    float lightIntensity = 1.0f;
    float ambientLightIntensity = 0.1f;
    float sphereRadius = 1.0f;
    bool raytraced = true;
    float cylinderRadius = 0.5f;
    int cylinderMode = 0;
    float pitch = 0.5f;
    float width = 0.25f;

    // Update MVP matrices and light position
    void updateMatrices(GLFWwindow *window, Camera &camera);

    // Call GL functions to set all uniforms for a given shader.
    // NOTE When introducing new uniforms, they must be manually added to this function as well
    void setUniforms(GLuint shaderProgram);
};

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
// TODO Destructors
struct DrawObject {
    GLuint vao = 0;
    GLuint vbo = 0;

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

// Helper functions to create DrawObjects from a set of input points
Mesh createSplineMesh(BSpline& spline, int samples, int segments, float radius);
Spheres createSpheres(std::vector<glm::vec3> &points);
Cylinders createCylinders(std::vector<glm::vec3> &points);

void bakeLightmap(GLuint *texture, Mesh &mesh, GLuint shaderProgram);

// Draw DrawObject with given shader and uniform values
void draw(DrawObject &object, GLuint shaderProgram, Uniforms &uniforms);
