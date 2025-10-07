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
    else if (mouse.rightButtonDown) {
        dist = (1.0f + 0.005f * mouse.dy) * dist;
    }
    dist = (1.0f - 0.25f * mouse.dscroll) * dist;
    dist = glm::clamp(dist, 1.0f, 500.0f);
}

void Uniforms::updateMatrices(GLFWwindow *window, Camera &camera) {
    // Create projection matrix
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    projection = glm::perspective(
        glm::radians(camera.fov),   // Field of view
        float(w) / float(h),        // Aspect ratio
        0.1f,                       // Near plane
        1000.0f                      // Far plane
    );
    // Create view matrix
    view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -camera.dist));
    view = glm::rotate(view, glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    // Create model matrix
    model = glm::mat4(1.0f);

    // Precompute light position in view space
    lightPos = view * glm::vec4(1.0f, 1.5f, 4.5f, 1.0f);
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

DrawObject createMesh(std::vector<float> &vertices, std::vector<unsigned int> &indices) {
    // Create buffers
    GLuint vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // Bind and fill IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    return DrawObject {
        vao,
        vbo,
        ibo,
        std::vector<float>(vertices),
        std::vector<unsigned int>(indices),
        unsigned(vertices.size() / 8),
        unsigned(indices.size()),
    };
}

DrawObject createTubeMesh(const BSpline& spline, int splineSamples = 50, int loopResolution = 8, float radius = 1.0f) {
    // Generate points along the spline
    std::vector<glm::vec3> splinePoints;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;

    float totalLength = spline.arcLength(1.0f);
    //std::cout << "Spline length: " << totalLength << std::endl;

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
            //normal *= -1.0f; // TODO
            //std::cout << "normal flipped" << std::endl;
        }
        normals.push_back(normal);
        glm::vec3 binormal = glm::normalize(glm::cross(tangent, normal));
        //std::cout << "binormal " << binormal.x << ", " << binormal.y << ", " << binormal.z << std::endl;
        //if (i > 0) std::cout << glm::dot(binormal, binormals.back()) << std::endl;
        //if (i > 0 && glm::dot(binormal, binormals.back()) < 0.0) {
        //    binormal *= -1.0f; // TODO
        //    std::cout << "binormal flipped" << std::endl;
        //}
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
            // TODO Formula for arbitrary width sheet with evenly rounded corners

            float angle = 2.0f * M_PI * float(j) / float(loopResolution);
            float d = glm::sin(angle);
            float n = glm::cos(angle);

            d = std::clamp(d, -0.125f, 0.125f); // TODO Capped circular
            //n = std::clamp(n, -0.125f, 0.125f);
            //d /= 5.0f; // TODO Oval
            //n /= 5.0f;
            //if (n > 0.0f) n += 0.5f;
            //else if (n < 0.0f) n -= 0.5f;
            //float d = (j % (loopResolution / 2) == 0) ? 0.0f : 1.0f;
            //float d = 1.0f;

            // TODO Rectangular with dull edge, even spacing
            //int a = j % (loopResolution / 2);
            //float d = (a == 0) ? 0.0f : 0.125f;
            //float n = -((a / (loopResolution / 2.0f - 1.0f)) * 2.0f - 1.0f);
            //if (j < loopResolution / 2) {
            //}
            //else {
            //    d *= -1.0f;
            //    n *= -1.0f;
            //}

            // Arrows
            //t = fmod(t, 0.1f);
            //float ar = 0.03f;
            //if (t > 2.0f * ar) {
            //    n *= 0.25f;
            //}
            //else if (t > ar) {
            //    n *= 0.5f;
            //}
            //else {
            //    n *= t / ar + 0.25f;
            //}

            glm::vec3 offset = radius * (d * binormal + n * normal);
            rings[i][j] = center + offset;
        }
    }

    // Generate triangles between consecutive rings
    unsigned int totalVertices = splineSamples * (loopResolution + 1);
    std::vector<float> vertices(totalVertices * 8);
    std::vector<unsigned int> indices;
    std::vector<float> distAroundRing(loopResolution + 1);
    // Fill positions and texture coordinates
    for (int i = 0; i < splineSamples; i++) {
        // First vertex
        int vertexIndex = (i * (loopResolution + 1)) * 8;
        int ringIndex = 0;
        glm::vec3 pos = rings[i][ringIndex];
        vertices[vertexIndex] = pos.x;
        vertices[vertexIndex + 1] = pos.y;
        vertices[vertexIndex + 2] = pos.z;
        glm::vec3 prevPos = pos;
        distAroundRing[0] = 0.0f;
        for (int j = 1; j <= loopResolution; j++) {
            int vertexIndex = (i * (loopResolution + 1) + j) * 8;
            int ringIndex = j % loopResolution;
            // Position
            glm::vec3 pos = rings[i][ringIndex];
            vertices[vertexIndex] = pos.x;
            vertices[vertexIndex + 1] = pos.y;
            vertices[vertexIndex + 2] = pos.z;
            // Normal will be filled later at indices 3, 4, 5
            distAroundRing[j] = distAroundRing[j - 1] + glm::distance(prevPos, pos);
            prevPos = pos;
        }
        float totalDist = distAroundRing[loopResolution];
        for (int j = 0; j <= loopResolution; j++) {
            int vertexIndex = (i * (loopResolution + 1) + j) * 8;
            // Texture coordinates
            float u = float(i) / float(splineSamples - 1);
            float v = distAroundRing[j] / totalDist;
            vertices[vertexIndex + 6] = 0.999f * u + 0.0005f;
            vertices[vertexIndex + 7] = 0.999f * v + 0.0005f;
        }
    }
    // Calculate averaged normals for each vertex
    std::vector<glm::vec3> vertexNormals(totalVertices, glm::vec3(0.0f));
    std::vector<int> normalCounts(totalVertices, 0);
    for (int i = 0; i < splineSamples - 1; i++) {
        for (int j = 0; j < loopResolution; j++) {
            // Vertex indices for the quad
            unsigned int v0 = i * (loopResolution + 1) + j;
            unsigned int v1 = i * (loopResolution + 1) + j + 1;
            unsigned int v1Base = i * (loopResolution + 1) + (j + 1) % loopResolution;
            unsigned int v2 = (i + 1) * (loopResolution + 1) + j;
            unsigned int v3 = (i + 1) * (loopResolution + 1) + j + 1;
            unsigned int v3Base = (i + 1) * (loopResolution + 1) + (j + 1) % loopResolution;

            // Get vertex positions
            glm::vec3 pos0 = rings[i][j];
            glm::vec3 pos1 = rings[i][(j + 1) % loopResolution];
            glm::vec3 pos2 = rings[i + 1][j];
            glm::vec3 pos3 = rings[i + 1][(j + 1) % loopResolution];

            // First triangle (v0, v1, v2)
            glm::vec3 normal1 = glm::normalize(glm::cross(pos1 - pos0, pos2 - pos0));
            indices.insert(indices.end(), {v0, v1, v2});

            // Accumulate normal for each vertex of first triangle
            vertexNormals[v0] += normal1;
            vertexNormals[v1Base] += normal1;
            vertexNormals[v2] += normal1;
            normalCounts[v0]++;
            normalCounts[v1Base]++;
            normalCounts[v2]++;

            // Second triangle (v1, v3, v2)
            glm::vec3 normal2 = glm::normalize(glm::cross(pos3 - pos1, pos2 - pos1));
            indices.insert(indices.end(), {v1, v3, v2});

            // Accumulate normal for each vertex of second triangle
            vertexNormals[v1Base] += normal2;
            vertexNormals[v3Base] += normal2;
            vertexNormals[v2] += normal2;
            normalCounts[v1Base]++;
            normalCounts[v3Base]++;
            normalCounts[v2]++;
        }
    }
    // Average the normals and store them in the vertices array
    for (int i = 0; i < totalVertices; i++) {
        int baseI = i;
        if (i % (loopResolution + 1) == loopResolution) baseI -= loopResolution;
        if (normalCounts[baseI] > 0) {
            glm::vec3 avgNormal = glm::normalize(vertexNormals[baseI] / float(normalCounts[baseI]));
            int normalIndex = i * 8 + 3;
            vertices[normalIndex] = avgNormal.x;
            vertices[normalIndex + 1] = avgNormal.y;
            vertices[normalIndex + 2] = avgNormal.z;
        }
    }

    return createMesh(vertices, indices);
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
        vao,
        vbo,
        0,
        vertices,
        std::vector<unsigned int>(),
        unsigned(vertices.size() / 3),
        0,
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
        vao,
        vbo,
        0,
        vertices,
        std::vector<unsigned int>(),
        unsigned(vertices.size() / 12),
        0,
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
    uniformLoc = glGetUniformLocation(shaderProgram, "drawTexture");
    glUniform1i(uniformLoc, uniforms.drawTexture);
    uniformLoc = glGetUniformLocation(shaderProgram, "lightIntensity");
    glUniform1f(uniformLoc, uniforms.lightIntensity);
    uniformLoc = glGetUniformLocation(shaderProgram, "ambientLightIntensity");
    glUniform1f(uniformLoc, uniforms.ambientLightIntensity);
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

    // Bind and draw
    glBindVertexArray(object.vao);
    glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
    if (object.nIndices > 0) {
        glDrawElements(GL_TRIANGLES, object.nIndices, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, object.nVertices);
    }
}
