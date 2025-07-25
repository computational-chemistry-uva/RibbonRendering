#include "gl.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glm/ext/scalar_constants.hpp>
#include "window.h"

// Set GL version
const char* glsl_version = "#version 130";

// Imgui setup
// NOTE Needs to be done after setting input callbacks!
void initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

struct Settings {
    Uniforms uniforms;
    bool drawWireframes = false;
    bool drawMesh = false;
    bool drawSpheres = true;
    bool drawCylinders = true;
};

std::vector<glm::vec3> nGonPoints(uint n) {
    std::vector<glm::vec3> points;
    for (int i = 0; i < n; ++i) {
        float angle = i / float(n) * 2.0f * glm::pi<float>();
        float y = 1.0f / glm::sqrt(3.0f) * cos(angle);
        float z = 1.0f / glm::sqrt(3.0f) * sin(angle);
        points.emplace_back(0.0f, y, z);
    }
    return points;
}

void settingsUI(Settings &settings) {
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    // Toggle visibility of objects
    ImGui::Text("Objects");
    ImGui::Indent();
    {
        ImGui::Checkbox("Mesh", &settings.drawMesh);
        ImGui::Checkbox("Spheres", &settings.drawSpheres);
        ImGui::Checkbox("Cylinders", &settings.drawCylinders);
    }
    ImGui::Unindent();

    // Settings that apply to all objects
    ImGui::Spacing();
    ImGui::Text("Draw settings");
    ImGui::Indent();
    {
        ImGui::Checkbox("Draw wireframes", &settings.drawWireframes);
        ImGui::Checkbox("Draw normals", &settings.uniforms.drawNormals);
        ImGui::SetNextItemWidth(128);
        if (settings.uniforms.drawNormals) ImGui::BeginDisabled();
        {
            ImGui::SliderFloat("Light", &settings.uniforms.lightIntensity, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        }
        if (settings.uniforms.drawNormals) ImGui::EndDisabled();
    }
    ImGui::Unindent();

    // Spheres parameters
    if (!settings.drawSpheres) ImGui::BeginDisabled();
    {
        ImGui::Spacing();
        ImGui::Text("Sphere impostors");
        ImGui::Indent();
        {
            ImGui::SetNextItemWidth(128);
            ImGui::SliderFloat("Radius##Sphere", &settings.uniforms.sphereRadius, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::Checkbox("Ray traced", &settings.uniforms.raytraced);
        }
        ImGui::Unindent();
    }
    if (!settings.drawSpheres) ImGui::EndDisabled();

    // Cylinders parameters
    if (!settings.drawCylinders) ImGui::BeginDisabled();
    ImGui::Spacing();
    ImGui::Text("Cylinder impostors");
    ImGui::Indent();
    {
        ImGui::SetNextItemWidth(128);
        ImGui::SliderFloat("Radius##Cylinder", &settings.uniforms.cylinderRadius, 0.05f, 0.25f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SetNextItemWidth(128);
        ImGui::Combo("Mode", &settings.uniforms.cylinderMode, "Simple\0Rounded\0Ribbon\0");
        if (settings.uniforms.cylinderMode != 2) ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(128);
        ImGui::SliderFloat("Pitch", &settings.uniforms.pitch, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SetNextItemWidth(128);
        ImGui::SliderFloat("Width", &settings.uniforms.width, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        if (settings.uniforms.cylinderMode != 2) ImGui::EndDisabled();
    }
    ImGui::Unindent();
    if (!settings.drawCylinders) ImGui::EndDisabled();

    ImGui::End();
}

int main() {
    // Initialize GL context and create window
    GLFWwindow *window = initWindow();
    if (!window) return 1;

    // Initialize ImGui
    initImGui(window);

    // Load and compile shaders
    Shaders shaders;

    // Create Objects
    std::vector<glm::vec3> points = nGonPoints(6);
    DrawObject mesh = createNGonMesh(points);
    DrawObject spheres = createSpheres(points);
    points.push_back(points[0]); // Close loop
    DrawObject cylinders = createCylinders(points);

    // Set default parameters
    Camera camera;
    Settings settings;

    // Create input struct and let GLFW access it
    MouseState mouse;
    glfwSetWindowUserPointer(window, &mouse);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Handle events
        glfwPollEvents();

        // Update mouse state
        mouse.update(window);

        // Update camera
        camera.update(mouse);
        settings.uniforms.updateMatrices(window, camera);

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        settingsUI(settings);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw objects
        glEnable(GL_CULL_FACE);
        glDepthRange(0.0, 1.0);
        if (settings.drawMesh) draw(mesh, shaders.meshProgram, settings.uniforms);
        if (settings.drawSpheres) draw(spheres, shaders.sphereProgram, settings.uniforms);
        if (settings.drawCylinders) draw(cylinders, shaders.cylinderProgram, settings.uniforms);
        if (settings.drawWireframes) {
            // Draw wireframes with no backface culling and on top of everything else
            glDisable(GL_CULL_FACE);
            glDepthRange(0.0, 0.0);
            if (settings.drawMesh) draw(mesh, shaders.meshWireframeProgram, settings.uniforms);
            if (settings.drawSpheres) draw(spheres, shaders.sphereWireframeProgram, settings.uniforms);
            if (settings.drawCylinders) draw(cylinders, shaders.cylinderWireframeProgram, settings.uniforms);
        }

        // Draw UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Finish frame
        glfwSwapBuffers(window);
    }

    // Cleanup
    destroyWindow(window);

    return 0;
}
