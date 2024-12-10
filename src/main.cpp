#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

namespace global {
    static struct MouseState {
        float x = 0.0f;
        float y = 0.0f;
        float scroll = 0.0f;
        float dx = 0.0f;
        float dy = 0.0f;
        float dscroll = 0.0f;
        bool leftButtonDown = false;

        void update(GLFWwindow *window) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            this->dx = float(x) - this->x;
            this->dy = float(y) - this->y;
            this->x = float(x);
            this->y = float(y);
            this-> dscroll = this->scroll;
            this->scroll = 0.0f;
        }
    } mouse;
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Register mouse release even when over UI
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        global::mouse.leftButtonDown = false;
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Only register click when not over UI
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        global::mouse.leftButtonDown = true;
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    global::mouse.scroll += yoffset;
}

std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile) {
        std::cerr << "Cannot read shader " << filePath << std::endl;
    }
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    return buffer.str();
}

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

GLuint createShaderProgram(
    const char* vertexPath,
    const char* geometryPath,
    const char* fragmentPath
) {
    std::string vertexSource = readShaderFile(vertexPath);
    std::string geometrySource = readShaderFile(geometryPath);
    std::string fragmentSource = readShaderFile(fragmentPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint geometryShader = compileShader(GL_GEOMETRY_SHADER, geometrySource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
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
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        return 1;
    }

    // Set GL version
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Viewport", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    GLuint shaderProgram = createShaderProgram(
        "../src/vertex.glsl",
        "../src/geometry.glsl",
        "../src/fragment.glsl"
    );

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);

    // Imgui setup
    // NOTE Needs to be done after setting input callbacks!
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup projection matrix
    windowResizeCallback(window, 1280, 720);

    // Cube vertex data: positions (x, y, z) and normals (nx, ny, nz)
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // Bottom-left
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // Bottom-right
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // Top-right
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, // Top-left

        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // Bottom-left
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // Bottom-right
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // Top-right
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, // Top-left

        // Left face
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, // Bottom-left
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, // Bottom-right
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, // Top-right
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, // Top-left

        // Right face
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, // Bottom-left
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, // Bottom-right
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, // Top-right
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, // Top-left

        // Top face
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // Bottom-left
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, // Bottom-right
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // Top-right
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, // Top-left

        // Bottom face
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, // Bottom-left
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, // Bottom-right
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, // Top-right
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f  // Top-left
    };

    // Indices for rendering the cube using triangle primitives
    unsigned int indices[] = {
        // Front face
        0, 1, 2,  2, 3, 0,
        // Back face
        4, 5, 6,  6, 7, 4,
        // Left face
        8, 9, 10, 10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 21, 22, 22, 23, 20
    };

    unsigned int n_indices = 36;

    // Create Vertex Array Object (VAO)
    GLuint VAO, VBO, IBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);
    // Bind VAO first
    glBindVertexArray(VAO);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Bind and fill IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float yaw = -45.0f;
    float pitch = 30.0f;
    float dist = 5.0f;
    float fov = 45.0f;
    int renderMode = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Get user input and window events
        glfwPollEvents();
        global::mouse.update(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0, 0});
        // ImGui::SetNextWindowSize({250, 100});
        ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        ImGui::SetNextItemWidth(80);
        ImGui::Combo("Rendering mode", &renderMode, "Shaded\0Normals\0");
        ImGui::End();

        if (global::mouse.leftButtonDown) {
            yaw += global::mouse.dx * 0.2f;
            pitch += global::mouse.dy * 0.2f;
            pitch = glm::clamp(pitch, -90.0f, 90.0f);
            yaw = fmod(yaw + 360.0f, 360.0f);
        }
        dist = (1.0f - 0.25f * global::mouse.dscroll) * dist;
        dist = glm::clamp(dist, 1.0f, 10.0f);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projection matrix
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        glm::mat4 projection = glm::perspective(
            glm::radians(fov),  // Field of view
            float(w) / float(h),  // Aspect ratio
            0.1f,                 // Near plane
            100.0f                // Far plane
        );
        // View matrix
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -dist));
        view = glm::rotate(view, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 viewPos = glm::inverse(view) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        // Model matrix
        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // Set uniforms
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3f(viewPosLoc, viewPos.x, viewPos.y, viewPos.z);
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        glUniform3f(lightPosLoc, 3.0f, 3.5f, 2.5f);
        GLint renderModeLoc = glGetUniformLocation(shaderProgram, "renderMode");
        glUniform1i(renderModeLoc, renderMode);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Finish frame
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
