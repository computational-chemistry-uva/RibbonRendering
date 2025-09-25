#include "window.h"
#include <iostream>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "input.h"

// Print GLFW errors
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Resize GL viewport on window resize
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Handle keyboard input
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Close window on Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // Ignore keypresses that are handled by ImGui
    if (ImGui::GetIO().WantCaptureKeyboard) return;
}

// Handle mouse input
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    MouseState *mouse = static_cast<MouseState *>(glfwGetWindowUserPointer(window));
    // Register mouse release even when over UI
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse->leftButtonDown = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        mouse->rightButtonDown = false;
    }
    // Only register click when not over UI
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse->leftButtonDown = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouse->rightButtonDown = true;
    }
}
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Only register scroll when not over UI
    if (ImGui::GetIO().WantCaptureMouse) return;
    MouseState *mouse = static_cast<MouseState *>(glfwGetWindowUserPointer(window));
    mouse->scroll += yoffset;
}

// Initialize GLFW, GLEW, and set callbacks
GLFWwindow* initWindow() {
    // Print GLFW errors
    glfwSetErrorCallback(glfw_error_callback);

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    // Set GL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // Enable MSAA (works with impostor shaders out of the box!)
    //glfwWindowHint(GLFW_SAMPLES, 16);

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_STENCIL_BITS, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // TODO Fixed size until viewport is dynamic
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Viewer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // Enable VSync
    glfwSwapInterval(1);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return nullptr;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Enable backface culling
    glEnable(GL_CULL_FACE);
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    windowResizeCallback(window, 1280, 720);

    return window;
}

void destroyWindow(GLFWwindow *window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
