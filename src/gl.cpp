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
#define LIGHTMAPPER_IMPLEMENTATION
#include "lightmapper.h"

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
    lightPos = view * glm::vec4(2.0f, 3.0f, 9.0f, 1.0f);
}

// Helper functions
void setUniform(GLuint shaderProgram, const char name[], const glm::mat4& v) {
    GLint loc = glGetUniformLocation(shaderProgram, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
}
void setUniform(GLuint shaderProgram, const char name[], const glm::vec3& v) {
    GLint loc = glGetUniformLocation(shaderProgram, name);
    glUniform3fv(loc, 1, glm::value_ptr(v));
}
void setUniform(GLuint shaderProgram, const char name[], float v) {
    GLint loc = glGetUniformLocation(shaderProgram, name);
    glUniform1f(loc, v);
}
void setUniform(GLuint shaderProgram, const char name[], int v) {
    GLint loc = glGetUniformLocation(shaderProgram, name);
    glUniform1i(loc, v);
}
void setUniform(GLuint shaderProgram, const char name[], bool v) {
    GLint loc = glGetUniformLocation(shaderProgram, name);
    glUniform1i(loc, v);
}

void Uniforms::setUniforms(GLuint shaderProgram) {
    setUniform(shaderProgram, "model", model);
    setUniform(shaderProgram, "view", view);
    setUniform(shaderProgram, "projection", projection);
    setUniform(shaderProgram, "lightPos", lightPos);
    setUniform(shaderProgram, "drawNormals", drawNormals);
    setUniform(shaderProgram, "drawTexture", drawTexture);
    setUniform(shaderProgram, "checkerboard", checkerboard);
    setUniform(shaderProgram, "lightIntensity", lightIntensity);
    setUniform(shaderProgram, "ambientLightIntensity", ambientLightIntensity);
    setUniform(shaderProgram, "sphereRadius", sphereRadius);
    setUniform(shaderProgram, "raytraced", raytraced);
    setUniform(shaderProgram, "cylinderRadius", cylinderRadius);
    setUniform(shaderProgram, "cylinderMode", cylinderMode);
    setUniform(shaderProgram, "pitch", pitch);
    setUniform(shaderProgram, "width", width);
}

Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices) {
    // Create buffers
    GLuint vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex), vertices.data(), GL_STATIC_DRAW);
    // Bind and fill IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    // TODO Note that vertex attributes should be changed here, or make it a function of Vertex
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, position));
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, normal));
    glEnableVertexAttribArray(1);
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, texCoord));
    glEnableVertexAttribArray(2);

    this->vao = vao;
    this->vbo = vbo;
    this->ibo = ibo;
    this->vertices = vertices;
    this->indices = indices;
}

Mesh createSplineMesh(BSpline& spline, int splineSamples = 50, int loopResolution = 8, float radius = 1.0f) {
    float totalLength = spline.arcLength(1.0f);
    //std::cout << "Spline length: " << totalLength << std::endl;

    // Sample points along the spline
    std::vector<glm::vec3> splinePoints;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;
    for (int i = 0; i < splineSamples; i++) {
        float targetLength = float(i) / float(splineSamples - 1) * totalLength;
        float t = spline.parameterFromArcLength(targetLength, totalLength);
        glm::vec3 point = spline.evaluate(t);
        splinePoints.push_back(point);
        glm::vec3 tangent = glm::normalize(spline.derivative(t));
        tangents.push_back(tangent);
        glm::vec3 normal = glm::normalize(spline.evaluateOrientation(t));
        // TODO When spline binormals flip, mesh ends up with flipped normals
        // TODO Points where normals flip need to be the same across LODs
        //if (i > 0 && glm::dot(normal, normals.back()) < 0.0) {
        //    normal *= -1.0f; // TODO
        //    std::cout << "normal flipped" << std::endl;
        //}
        normals.push_back(normal);
        glm::vec3 binormal = glm::normalize(glm::cross(tangent, normal));
        //if (i > 0 && glm::dot(binormal, binormals.back()) < 0.0) {
        //    binormal *= -1.0f; // TODO
        //    std::cout << "binormal flipped" << std::endl;
        //}
        binormals.push_back(binormal);
    }

    // Create cross-section circles for each sample
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

            // NOTE Capped circular
            d = glm::clamp(d, -0.125f, 0.125f);
            //n = glm::clamp(n, -0.125f, 0.125f);

            // NOTE Oval
            //d /= 5.0f;
            //n /= 5.0f;
            //if (n > 0.0f) n += 0.5f;
            //else if (n < 0.0f) n -= 0.5f;
            //float d = (j % (loopResolution / 2) == 0) ? 0.0f : 1.0f;
            //float d = 1.0f;

            // NOTE Rectangular with dull edge, even spacing
            //int a = j % (loopResolution / 2);
            //float d = (a == 0) ? 0.0f : 0.125f;
            //float n = -((a / (loopResolution / 2.0f - 1.0f)) * 2.0f - 1.0f);
            //if (j < loopResolution / 2) {
            //}
            //else {
            //    d *= -1.0f;
            //    n *= -1.0f;
            //}

            // TODO Arrows
            if (i < float(2) / float(spline.getControlPoints().size()) * splineSamples) {
                n *= 0.25f;
            }
            else if (i < float(12) / float(spline.getControlPoints().size()) * splineSamples) {
                float l = 2.0f / spline.getControlPoints().size();
                t = fmod(t, l);
                float ar = 1.0f / spline.getControlPoints().size();
                if (t > 2.0f * ar) {
                    n *= 0.25f;
                }
                else if (t > ar) {
                    n *= 0.5f;
                }
                else {
                    n *= t / ar + 0.25f;
                }
            }

            glm::vec3 offset = radius * (d * binormal + n * normal);
            rings[i][j] = center + offset;
        }
    }

    // Generate triangles between consecutive rings
    unsigned int totalVertices = splineSamples * (loopResolution + 1);
    std::vector<MeshVertex> vertices(totalVertices);
    std::vector<unsigned int> indices;
    std::vector<float> distAroundRing(loopResolution + 1);
    // Fill positions and texture coordinates
    for (int i = 0; i < splineSamples; i++) {
        // First vertex
        int vertexIndex = (i * (loopResolution + 1));
        int ringIndex = 0;
        glm::vec3 pos = rings[i][ringIndex];
        vertices[vertexIndex].position = pos;
        glm::vec3 prevPos = pos;
        distAroundRing[0] = 0.0f;
        for (int j = 1; j <= loopResolution; j++) {
            int vertexIndex = (i * (loopResolution + 1) + j);
            int ringIndex = j % loopResolution;
            // Position
            glm::vec3 pos = rings[i][ringIndex];
            vertices[vertexIndex].position = pos;
            // Normal will be filled later at indices 3, 4, 5
            distAroundRing[j] = distAroundRing[j - 1] + glm::distance(prevPos, pos);
            prevPos = pos;
        }
        float totalDist = distAroundRing[loopResolution];
        for (int j = 0; j <= loopResolution; j++) {
            int vertexIndex = (i * (loopResolution + 1) + j);
            // Texture coordinates
            float u = float(i) / float(splineSamples - 1);
            float v = distAroundRing[j] / totalDist;
            u = 0.999f * u + 0.0005f;
            v = 0.999f * v + 0.0005f;
            vertices[vertexIndex].texCoord = glm::vec2(u, v);
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
            vertices[i].normal = avgNormal;
        }
    }

    return Mesh(vertices, indices);
}

Spheres::Spheres(std::vector<SphereVertex> vertices) {
    // Create buffers
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    this->vao = vao;
    this->vbo = vbo;
    this->vertices = vertices;
}

// NOTE We have to upload the same vertex multiple times so the vertex shader can displace them and form a quad.
//      Can't reuse the same vertex with indexed drawing because of the way gl_VertexID works.
Spheres createSpheres(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<SphereVertex> vertices;
    for (int i = 0; i < points.size(); i++) {
        for (int j = 0; j < 6; j++) {
            vertices.push_back(SphereVertex { points[i] });
        }
    }
    return Spheres(vertices);
};

Cylinders::Cylinders(std::vector<CylinderVertex> vertices) {
    // Create buffers
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // Bind VAO first
    glBindVertexArray(vao);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CylinderVertex), vertices.data(), GL_STATIC_DRAW);
    // A position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CylinderVertex), (void*)offsetof(CylinderVertex, aPos));
    glEnableVertexAttribArray(0);
    // B position plane normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CylinderVertex), (void*)(offsetof(CylinderVertex, bPos)));
    glEnableVertexAttribArray(1);
    // A cut plane normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CylinderVertex), (void*)(offsetof(CylinderVertex, aCPN)));
    glEnableVertexAttribArray(2);
    // B cut plane normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CylinderVertex), (void*)(offsetof(CylinderVertex, bCPN)));
    glEnableVertexAttribArray(3);
    // Start dir attribute
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(CylinderVertex), (void*)(offsetof(CylinderVertex, startDir)));
    glEnableVertexAttribArray(4);

    this->vao = vao;
    this->vbo = vbo;
    this->vertices = vertices;
}

// TODO Varying cylinder width
// TODO This and the cylinder shaders could be split and optimized for the simple cylinder or helix case.
//      For example, helices do not form splines so they don't need cut planes.
//      Then they also only need one cap quad because the other always faces away from the camera.
Cylinders createCylinders(std::vector<glm::vec3> &points) {
    // Create vertex data
    std::vector<CylinderVertex> vertices;
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
        CylinderVertex v { a, b, aCPN, bCPN, startDir };
        for (int j = 0; j < 18; j++) {
            vertices.push_back(v);
        }
    }
    return Cylinders(vertices);
};

// See https://github.com/ands/lightmapper
void bakeLightmap(GLuint *texture, Mesh &mesh, GLuint shaderProgram) {
    // TODO Runtime controls for re-baking
    // TODO Credit lightmapper in README (check license)
    // TODO Add license
    // TODO Compute texture size dynamically
    // TODO Pack into square texture

    const int w = 2048;
    const int h = 32;

    // Create lightmapper context
    lm_context *ctx = lmCreate(
            64,               // Hemisphere resolution (power of two, max=512)
            0.001f, 100.0f,   // zNear, zFar of hemisphere cameras
            1.0f, 1.0f, 1.0f, // Background color (white for ambient occlusion)
            2, 0.01f,         // Lightmap interpolation threshold (small differences are interpolated rather than sampled)
            0.0f);            // Modifier for camera-to-surface distance for hemisphere rendering
    if (!ctx) {
        fprintf(stderr, "Error: Could not initialize lightmapper.\n");
        return;
    }

    // Create texture if it doesn't exist yet
    if (*texture == 0) {
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Loop around
    }
    unsigned char emissive[] = { 0, 0, 0, 255 }; // Initial texture color
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, emissive);

    // Create buffer to store lightmap data in
    float data[w * h * 4] = { 0.0f };

    for (int bounce = 0; bounce < 1; bounce++) {
        // Clear buffer
        memset(data, 0.0f, sizeof(float) * w * h * 3);
        lmSetTargetLightmap(ctx, data, w, h, 4);

        // Set mesh data
        char *vertexData = reinterpret_cast<char*>(mesh.vertices.data());
        lmSetGeometry(ctx, NULL,
                LM_FLOAT, vertexData + offsetof(MeshVertex, position), sizeof(MeshVertex),
                LM_FLOAT, vertexData + offsetof(MeshVertex, normal), sizeof(MeshVertex),
                LM_FLOAT, vertexData + offsetof(MeshVertex, texCoord), sizeof(MeshVertex),
                mesh.indices.size(), LM_UNSIGNED_INT, mesh.indices.data());

        // Set GL drawing settings for lightmapper to work properly
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glShadeModel(GL_SMOOTH);
        glDepthRange(0.0, 1.0);

        // Bake the lightmap by rendering the scene for each hemisphere point
        int vp[4];
        glm::mat4 view, proj;
        double lastUpdateTime = 0.0;
        int i = 0;
        while (lmBegin(ctx, vp, &view[0][0], &proj[0][0])) {
            // Render to lightmapper framebuffer
            glViewport(vp[0], vp[1], vp[2], vp[3]);

            // Set uniforms
            Uniforms uniforms;
            uniforms.model = glm::mat4(1.0f);
            uniforms.view = view;
            uniforms.projection = proj;
            uniforms.lightIntensity = 0.0f; // Point light contribution should not be baked in
                                            //uniforms.lightPos = view * glm::vec4(1.0f, 1.5f, 4.5f, 1.0f);
            uniforms.ambientLightIntensity = 1.0f; // Self-irradiance in subsequent passes
            GLint uniformLoc = glGetUniformLocation(shaderProgram, "lightmap");
            glUniform1i(uniformLoc, 0);

            // Draw scene
            glBindTexture(GL_TEXTURE_2D, *texture);
            draw(mesh, shaderProgram, uniforms);

            // Display progress
            double time = glfwGetTime();
            if (time - lastUpdateTime > 0.01f) {
                lastUpdateTime = time;
                printf("\r%6.2f%%", lmProgress(ctx) * 100.0f);
                fflush(stdout);
            }

            lmEnd(ctx);
            i++;
        }
        printf("\rFinished baking %d triangles (%d iterations).\n", int(mesh.indices.size()) / 3, i);

        // Postprocess texture
        float temp[w * h * 4] = { 0.0f };
        lmImageDilate(data, temp, w, h, 4);
        lmImageSmooth(temp, data, w, h, 4);

        // Upload result
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, data);

        // Save result to a file
        // NOTE For debugging. Not necessary
        lmImagePower(data, w, h, 4, 1.0f / 2.2f, 0x7); // Gamma correct color channels
        if (lmImageSaveTGAf("lightmap.tga", data, w, h, 4, 1.0f)) {
            printf("Saved lightmap.tga\n");
        }
    }

    lmDestroy(ctx);
}

void Mesh::draw() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Spheres::draw() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

void Cylinders::draw() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

// TODO Make this a DrawObject member function?
void draw(DrawObject &object, GLuint shaderProgram, Uniforms &uniforms) {
    // Use shader
    glUseProgram(shaderProgram);

    // Set uniforms
    uniforms.setUniforms(shaderProgram);

    // Draw object
    object.draw();
}
