#include "gl.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

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

// TODO Add note about index-based rendering with vertex shaders

DrawObject createMesh(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<float> vertices;
    for (int i = 0; i < points.size(); i++) {
        vertices.push_back(points[i].x);
        vertices.push_back(points[i].y);
        vertices.push_back(points[i].z);
        // TODO Normals
        int a = std::max(i - 1, 0);
        int b = std::min(i + 1, 2);
        glm::vec3 v = glm::normalize(points[b] - points[a]);
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
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
};

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

DrawObject createCylinders(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<float> vertices;
    for (int i = 0; i < points.size() - 1; i++) {
        glm::vec3 a = points[i];
        glm::vec3 b = points[i + 1];
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
        // TODO For helices, only draw one cap quad
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // B position plane normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // A cut plane normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // B cut plane normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

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
    uniformLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3f(uniformLoc, uniforms.viewPos.x, uniforms.viewPos.y, uniforms.viewPos.z);
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
