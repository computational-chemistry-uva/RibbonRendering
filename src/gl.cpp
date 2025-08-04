#include "gl.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

void Camera::update(MouseState mouse) {
    if (mouse.leftButtonDown) {
        yaw += mouse.dx * 0.2f;
        pitch += mouse.dy * 0.2f;
        pitch = glm::clamp(pitch, -90.0f, 90.0f);
        yaw = fmod(yaw + 360.0f, 360.0f);
    }
    dist = (1.0f - 0.25f * mouse.dscroll) * dist;
    dist = glm::clamp(dist, 1.0f, 10.0f);
}

void Uniforms::updateMatrices(GLFWwindow *window, Camera &camera) {
    // Create projection matrix
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    projection = glm::perspective(
        glm::radians(camera.fov),   // Field of view
        float(w) / float(h),        // Aspect ratio
        0.1f,                       // Near plane
        100.0f                      // Far plane
    );
    // Create view matrix
    view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -camera.dist));
    view = glm::rotate(view, glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    // Create model matrix
    model = glm::mat4(1.0f);

    // Precompute light position in view space
    lightPos = view * glm::vec4(3.0f, 3.5f, 2.5f, 1.0f);
}

// Helper function to read shader from file
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile) {
        std::cerr << "Cannot read shader " << filePath << std::endl;
    }
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    return buffer.str();
}

// Helper function to (try to) compile shader
GLuint compileShader(GLenum type, const std::string& source) {
    const char* sourcePtr = source.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexSource = readShaderFile(vertexPath);
    std::string fragmentSource = readShaderFile(fragmentPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

DrawObject createMesh(std::vector<float> &vertices) {
    // Create buffers
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return DrawObject {
        vbo,
            vao,
            unsigned(vertices.size() / 6)
    };
}

DrawObject createNGonMesh(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<float> vertices;
    for (int i = 1; i < points.size() - 1; i++) {
        glm::vec3 a = points[0];
        glm::vec3 b = points[i];
        glm::vec3 c = points[i + 1];
        glm::vec3 normal = glm::cross(b - a, c - a);
        vertices.push_back(a.x);
        vertices.push_back(a.y);
        vertices.push_back(a.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
        vertices.push_back(b.x);
        vertices.push_back(b.y);
        vertices.push_back(b.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
        vertices.push_back(c.x);
        vertices.push_back(c.y);
        vertices.push_back(c.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
    }
    return createMesh(vertices);
};

DrawObject createTubeMesh(const BSpline& spline, int splineSamples = 50, int loopResolution = 8, float radius = 1.0f) {
    std::vector<float> vertices;
    
    // Generate points along the spline
    std::vector<glm::vec3> splinePoints;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;

    float totalLength = spline.arcLength(1.0f);
    
    for (int i = 0; i < splineSamples; i++) {
        float targetLength = float(i) / float(splineSamples - 1) * totalLength;
        float t = spline.parameterFromArcLength(targetLength, totalLength);
        glm::vec3 point = spline.evaluate(t);
        //std::cout << "point " << point.x << ", " << point.y << ", " << point.z << std::endl;
        splinePoints.push_back(point);
        glm::vec3 tangent = glm::normalize(spline.derivative(t));
        //std::cout << "tangent " << tangent.x << ", " << tangent.y << ", " << tangent.z << std::endl;
        tangents.push_back(tangent);
        glm::vec3 normal = glm::normalize(spline.evaluateOrientation(t));
        //std::cout << "normal " << normal.x << ", " << normal.y << ", " << normal.z << std::endl;
        //if (i > 0) std::cout << glm::dot(normal, normals.back()) << std::endl;
        if (i > 0 && glm::dot(normal, normals.back()) < 0.0) {
            normal *= -1.0f;
            std::cout << "normal flipped" << std::endl;
        }
        normals.push_back(normal);
        glm::vec3 binormal = glm::normalize(glm::cross(tangent, normal));
        //std::cout << "binormal " << binormal.x << ", " << binormal.y << ", " << binormal.z << std::endl;
        //if (i > 0) std::cout << glm::dot(binormal, binormals.back()) << std::endl;
        if (i > 0 && glm::dot(binormal, binormals.back()) < 0.0) {
            binormal *= -1.0f;
            std::cout << "binormal flipped" << std::endl;
        }
        //std::cout << std::endl;
        binormals.push_back(binormal);
    }
    
    // Create cross-section circles for each spline point
    std::vector<std::vector<glm::vec3>> rings(splineSamples);
    
    for (int i = 0; i < splineSamples; i++) {
        float t = float(i) / float(splineSamples - 1);
        glm::vec3 center = splinePoints[i];
        glm::vec3 tangent = tangents[i];
        glm::vec3 normal = normals[i];
        glm::vec3 binormal = binormals[i];

        // Generate circle points using interpolated orientation
        rings[i].resize(loopResolution);
        for (int j = 0; j < loopResolution; j++) {
            float angle = 2.0f * M_PI * float(j) / float(loopResolution);
            float d = glm::sin(angle);
            float n = glm::cos(angle);
            n = std::clamp(n, -0.25f, 0.25f);
            glm::vec3 offset = radius * (d * binormal + n * normal);
            rings[i][j] = center + offset;
        }
    }
    
    // Generate triangles between consecutive rings
    for (int i = 0; i < splineSamples - 1; i++) {
        for (int j = 0; j < loopResolution; j++) {
            int next_j = (j + 1) % loopResolution;
            
            // Current ring vertices
            glm::vec3 v0 = rings[i][j];
            glm::vec3 v1 = rings[i][next_j];
            glm::vec3 v2 = rings[i + 1][j];
            glm::vec3 v3 = rings[i + 1][next_j];
            
            // First triangle (v0, v1, v2)
            glm::vec3 normal1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            vertices.insert(vertices.end(), {v0.x, v0.y, v0.z, normal1.x, normal1.y, normal1.z});
            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal1.x, normal1.y, normal1.z});
            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal1.x, normal1.y, normal1.z});
            
            // Second triangle (v1, v3, v2)
            glm::vec3 normal2 = glm::normalize(glm::cross(v3 - v1, v2 - v1));
            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal2.x, normal2.y, normal2.z});
            vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal2.x, normal2.y, normal2.z});
            vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal2.x, normal2.y, normal2.z});
        }
    }
    
    return createMesh(vertices);
}

// NOTE We have to upload the same vertex multiple times so the vertex shader can displace them and form a quad.
//      Can't reuse the same vertex with indexed drawing because of the way gl_VertexID works.
DrawObject createSpheres(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<float> vertices;
    for (int i = 0; i < points.size(); i++) {
        for (int j = 0; j < 6; j++) {
            vertices.push_back(points[i].x);
            vertices.push_back(points[i].y);
            vertices.push_back(points[i].z);
        }
    }

    // Create buffers
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return DrawObject {
        vbo,
        vao,
        unsigned(vertices.size() / 3)
    };
};

// TODO Varying cylinder width
// TODO This and the cylinder shaders could be split and optimized for the simple cylinder or helix case.
//      For example, helices do not form splines so they don't need cut planes.
//      Then they also only need one cap quad because the other always faces away from the camera.
DrawObject createCylinders(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<float> vertices;
    for (int i = 0; i < points.size() - 1; i++) {
        glm::vec3 a = points[i];
        glm::vec3 b = points[i + 1];
        // Calculate cut plane normals
        glm::vec3 aCPN, bCPN;
        if (i < 1) {
            aCPN = glm::normalize(b - a);
        }
        else {
            aCPN = glm::normalize(glm::normalize(b - a) - glm::normalize(points[i - 1] - a));
        }
        if (i > points.size() - 3) {
            bCPN = glm::normalize(b - a);
        }
        else {
            bCPN = glm::normalize(glm::normalize(points[i + 2] - b) - glm::normalize(a - b));
        }
        // TODO Compute meaningful value for startDir
        //      It should be perpendicular to the cylinder axis!
        //      Currently this will produce artifacts if a cylinder is pointing in the x direction
        glm::vec3 startDir(1.0f, 0.0f, 0.0f);
        for (int j = 0; j < 18; j++) {
            vertices.push_back(a.x);
            vertices.push_back(a.y);
            vertices.push_back(a.z);
            vertices.push_back(b.x);
            vertices.push_back(b.y);
            vertices.push_back(b.z);
            vertices.push_back(aCPN.x);
            vertices.push_back(aCPN.y);
            vertices.push_back(aCPN.z);
            vertices.push_back(bCPN.x);
            vertices.push_back(bCPN.y);
            vertices.push_back(bCPN.z);
            vertices.push_back(startDir.x);
            vertices.push_back(startDir.y);
            vertices.push_back(startDir.z);
        }
    }

    // Create buffers
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // A position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // B position plane normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // A cut plane normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // B cut plane normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // Start dir attribute
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(4);

    return DrawObject {
        vbo,
        vao,
        unsigned(vertices.size() / 12)
    };
};

void draw(DrawObject &object, GLuint shaderProgram, Uniforms &uniforms) {
    // Use shader
    glUseProgram(shaderProgram);

    // Set uniforms
    GLint uniformLoc;
    uniformLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(uniforms.model));
    uniformLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(uniforms.view));
    uniformLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(uniforms.projection));
    uniformLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3f(uniformLoc, uniforms.lightPos.x, uniforms.lightPos.y, uniforms.lightPos.z);
    uniformLoc = glGetUniformLocation(shaderProgram, "drawNormals");
    glUniform1i(uniformLoc, uniforms.drawNormals);
    uniformLoc = glGetUniformLocation(shaderProgram, "lightIntensity");
    glUniform1f(uniformLoc, uniforms.lightIntensity);
    uniformLoc = glGetUniformLocation(shaderProgram, "sphereRadius");
    glUniform1f(uniformLoc, uniforms.sphereRadius);
    uniformLoc = glGetUniformLocation(shaderProgram, "raytraced");
    glUniform1i(uniformLoc, uniforms.raytraced);
    uniformLoc = glGetUniformLocation(shaderProgram, "cylinderRadius");
    glUniform1f(uniformLoc, uniforms.cylinderRadius);
    uniformLoc = glGetUniformLocation(shaderProgram, "cylinderMode");
    glUniform1i(uniformLoc, uniforms.cylinderMode);
    uniformLoc = glGetUniformLocation(shaderProgram, "width");
    glUniform1f(uniformLoc, uniforms.width);
    uniformLoc = glGetUniformLocation(shaderProgram, "pitch");
    glUniform1f(uniformLoc, uniforms.pitch);

    // Bind VAO
    glBindVertexArray(object.vao);

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, object.nVertices);
}
