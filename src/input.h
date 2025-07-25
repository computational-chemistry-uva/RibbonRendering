#pragma once

#include <GLFW/glfw3.h>

// Struct to keep track of mouse input
// d values indicate the change since the previous frame
struct MouseState {
    float x = 0.0f;
    float y = 0.0f;
    float scroll = 0.0f;
    float dx = 0.0f;
    float dy = 0.0f;
    float dscroll = 0.0f;
    bool leftButtonDown = false;

    // Update mouse position and change since last frame
    void update(GLFWwindow *window) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        this->dx = float(x) - this->x;
        this->dy = float(y) - this->y;
        this->x = float(x);
        this->y = float(y);
        this->dscroll = this->scroll;
        this->scroll = 0.0f;
    }
};
