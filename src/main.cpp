#include "gl.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <iostream>

// NOTE This currently does not handle deletion of GL resources, they are kept alive until termination of the program.

// Struct to keep track of mouse input
// d values indicate the change since the previous frame
namespace global {
    static struct MouseState {
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
    } mouse;
};

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
    // Register mouse release even when over UI
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        global::mouse.leftButtonDown = false;
    }
    // Only register click when not over UI
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        global::mouse.leftButtonDown = true;
    }
}
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Only register scroll when not over UI
    if (ImGui::GetIO().WantCaptureMouse) return;
    global::mouse.scroll += yoffset;
}

int main() {
    // Print GLFW errors
    glfwSetErrorCallback(glfw_error_callback);

    // Initialize GLFW
    if (!glfwInit()) {
        return 1;
    }

    // Set GL version
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // Enable MSAA (works with impostor shaders out of the box!)
    //glfwWindowHint(GLFW_SAMPLES, 16);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Viewer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    // Enable VSync
    glfwSwapInterval(1);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return 1;
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

    // Load and compile shaders
    GLuint meshWireframeProgram = createShaderProgram(
        "../src/mesh_vertex.glsl",
        "../src/wireframe_fragment.glsl"
    );
    GLuint meshProgram = createShaderProgram(
        "../src/mesh_vertex.glsl",
        "../src/mesh_fragment.glsl"
    );
    GLuint sphereWireframeProgram = createShaderProgram(
        "../src/sphere_vertex.glsl",
        "../src/wireframe_fragment.glsl"
    );
    GLuint sphereProgram = createShaderProgram(
        "../src/sphere_vertex.glsl",
        "../src/sphere_fragment.glsl"
    );
    GLuint cylinderWireframeProgram = createShaderProgram(
        "../src/cylinder_vertex.glsl",
        "../src/wireframe_fragment.glsl"
    );
    GLuint cylinderProgram = createShaderProgram(
        "../src/cylinder_vertex.glsl",
        "../src/cylinder_fragment.glsl"
    );

    // Create Objects
    std::vector<glm::vec3> points = {
        glm::vec3(0.0f, 0.0f, 0.5f),
        glm::vec3(0.0f, 0.0f, -0.5f),
        glm::vec3(0.0f, 0.5f, -1.0f),
    };
    DrawObject mesh = createMesh(points);
    DrawObject spheres = createSpheres(points);
    DrawObject cylinders = createCylinders(points);

    // Set default parameters
    Uniforms uniforms;
    uniforms.drawNormals = false;
    uniforms.lightIntensity = 1.0f;
    uniforms.sphereRadius = 0.25f;
    uniforms.raytraced = true;
    uniforms.cylinderRadius = 0.1f;
    uniforms.cylinderMode = 2;
    uniforms.pitch = 0.5f;
    uniforms.width = 0.25f;
    float camYaw = -45.0f;
    float camPitch = 30.0f;
    float camDist = 5.0f;
    float fov = 45.0f;
    bool drawWireframes = true;
    bool drawMesh = false;
    bool drawSpheres = false;
    bool drawCylinders = true;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Handle events
        glfwPollEvents();

        // Update mouse state
        global::mouse.update(window);

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({0, 0});
        ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
        {
            // Toggle visibility of objects
            ImGui::Text("Objects");
            ImGui::Indent();
            {
                ImGui::Checkbox("Mesh", &drawMesh);
                ImGui::Checkbox("Spheres", &drawSpheres);
                ImGui::Checkbox("Cylinders", &drawCylinders);
            }
            ImGui::Unindent();

            // Settings that apply to all objects
            ImGui::Spacing();
            ImGui::Text("Draw settings");
            ImGui::Indent();
            {
                ImGui::Checkbox("Draw wireframes", &drawWireframes);
                ImGui::Checkbox("Draw normals", &uniforms.drawNormals);
                ImGui::SetNextItemWidth(128);
                if (uniforms.drawNormals) ImGui::BeginDisabled();
                {
                    ImGui::SliderFloat("Light", &uniforms.lightIntensity, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                }
                if (uniforms.drawNormals) ImGui::EndDisabled();
            }
            ImGui::Unindent();

            // Spheres parameters
            if (!drawSpheres) ImGui::BeginDisabled();
            {
                ImGui::Spacing();
                ImGui::Text("Sphere impostors");
                ImGui::Indent();
                {
                    ImGui::SetNextItemWidth(128);
                    ImGui::SliderFloat("Radius##Sphere", &uniforms.sphereRadius, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                    ImGui::Checkbox("Ray traced", &uniforms.raytraced);
                }
                ImGui::Unindent();
            }
            if (!drawSpheres) ImGui::EndDisabled();

            // Cylinders parameters
            if (!drawCylinders) ImGui::BeginDisabled();
            ImGui::Spacing();
            ImGui::Text("Cylinder impostors");
            ImGui::Indent();
            {
                ImGui::SetNextItemWidth(128);
                ImGui::SliderFloat("Radius##Cylinder", &uniforms.cylinderRadius, 0.05f, 0.25f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                ImGui::SetNextItemWidth(128);
                ImGui::Combo("Mode", &uniforms.cylinderMode, "Simple\0Rounded\0Ribbon\0");
                if (uniforms.cylinderMode != 2) ImGui::BeginDisabled();
                ImGui::SetNextItemWidth(128);
                ImGui::SliderFloat("Pitch", &uniforms.pitch, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                ImGui::SetNextItemWidth(128);
                ImGui::SliderFloat("Width", &uniforms.width, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                if (uniforms.cylinderMode != 2) ImGui::EndDisabled();
            }
            ImGui::Unindent();
            if (!drawCylinders) ImGui::EndDisabled();
        }
        ImGui::End();

        // Update camera parameters based on mouse state
        if (global::mouse.leftButtonDown) {
            camYaw += global::mouse.dx * 0.2f;
            camPitch += global::mouse.dy * 0.2f;
            camPitch = glm::clamp(camPitch, -90.0f, 90.0f);
            camYaw = fmod(camYaw + 360.0f, 360.0f);
        }
        camDist = (1.0f - 0.25f * global::mouse.dscroll) * camDist;
        camDist = glm::clamp(camDist, 1.0f, 10.0f);

        // Create projection matrix
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        uniforms.projection = glm::perspective(
            glm::radians(fov),   // Field of view
            float(w) / float(h), // Aspect ratio
            0.1f,                // Near plane
            100.0f               // Far plane
        );
        // Create view matrix
        uniforms.view = glm::mat4(1.0f);
        uniforms.view = glm::translate(uniforms.view, glm::vec3(0.0f, 0.0f, -camDist));
        uniforms.view = glm::rotate(uniforms.view, glm::radians(camPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        uniforms.view = glm::rotate(uniforms.view, glm::radians(camYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        // Create model matrix
        uniforms.model = glm::mat4(1.0f);

        // Precompute light position in view space
        uniforms.lightPos = uniforms.view * glm::vec4(3.0f, 3.5f, 2.5f, 1.0f);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw objects
        glEnable(GL_CULL_FACE);
        glDepthRange(0.0, 1.0);
        if (drawMesh) draw(mesh, meshProgram, uniforms);
        if (drawSpheres) draw(spheres, sphereProgram, uniforms);
        if (drawCylinders) draw(cylinders, cylinderProgram, uniforms);
        if (drawWireframes) {
            // Draw wireframes with no backface culling and on top of everything else
            glDisable(GL_CULL_FACE);
            glDepthRange(0.0, 0.0);
            if (drawMesh) draw(mesh, meshWireframeProgram, uniforms);
            if (drawSpheres) draw(spheres, sphereWireframeProgram, uniforms);
            if (drawCylinders) draw(cylinders, cylinderWireframeProgram, uniforms);
        }

        // Draw UI
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
