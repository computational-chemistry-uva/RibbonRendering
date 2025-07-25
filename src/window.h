#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Print GLFW errors
static void glfw_error_callback(int error, const char* description);

// Resize GL viewport on window resize
void windowResizeCallback(GLFWwindow* window, int width, int height);

// Handle keyboard input
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Handle mouse input
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Initialize GLFW, GLEW, and set callbacks
GLFWwindow* initWindow();

// Clean up resources
void destroyWindow(GLFWwindow *window);
