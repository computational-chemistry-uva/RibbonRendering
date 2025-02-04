#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// Simple struct to help with drawing
struct DrawObject {
    GLuint vbo;
    GLuint vao;
    unsigned int nVertices;
};

// Uniform data to send to shaders
// TODO Get rid of viewPos and update vertex shaders
// TODO Some of these should be vertex attributes so that multiple instances
//      with different parameters can be drawn in one draw call
struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 viewPos;
    glm::vec3 lightPos;
    bool drawNormals;
    float lightIntensity;
    float sphereRadius;
    bool raytraced;
    float cylinderRadius;
    int cylinderMode;
    float pitch;
    float width;
};

// Load vertex and fragment shader from files and compile them into a shader program
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);

// Helper functions to create DrawObjects from a set of input points
DrawObject createMesh(std::vector<glm::vec3> &points) ;
DrawObject createSpheres(std::vector<glm::vec3> &points) ;
DrawObject createCylinders(std::vector<glm::vec3> &points) ;

// Draw DrawObject with given shader and uniform values
void draw(DrawObject &object, GLuint shaderProgram, Uniforms &uniforms);
