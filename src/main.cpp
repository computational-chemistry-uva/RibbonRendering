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
    bool drawMesh = true;
    bool drawSpheres = false;
    bool drawCylinders = false;
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

    // Set default parameters
    Camera camera;
    Settings settings;

    // Create input struct and let GLFW access it
    MouseState mouse;
    glfwSetWindowUserPointer(window, &mouse);

    // Create spline
    std::vector<glm::vec3> controlPoints = {
        //glm::vec3(0.0f, -2.0f, 0.0f),
        //glm::vec3(2.0f, -1.0f, 1.0f),
        //glm::vec3(-1.0f, 0.0f, 1.0f),
        //glm::vec3(2.0f, 1.0f, -1.0f),
        //glm::vec3(1.0f, 2.0f, 2.0f)

        //glm::vec3(-2.0f, -2.0f, 0.0f),
        //glm::vec3(0.0f, -2.0f, 0.0f),
        //glm::vec3(2.0f, -2.0f, 0.0f),
        //glm::vec3(2.0f, 0.0f, 0.0f),
        //glm::vec3(0.0f, 0.0f, 0.0f),
        //glm::vec3(-2.0f, 0.0f, 0.0f),
        //glm::vec3(-2.0f, 2.0f, 0.0f),
        //glm::vec3(0.0f, 2.0f, 0.0f),
        //glm::vec3(2.0f, 2.0f, 0.0f),

        glm::vec3( 4.047, -12.839,  16.901),
        glm::vec3( 5.757, -11.521,  13.850),
        glm::vec3( 9.465, -10.668,  13.672),
        glm::vec3(11.204,  -8.935,  10.767),
        glm::vec3(14.831,  -9.075,   9.617),
        glm::vec3(15.122,  -5.348,   8.865),
        glm::vec3(13.127,  -2.024,   8.869),
        glm::vec3(12.305,  -2.325,   5.169),
        glm::vec3(10.923,  -5.730,   5.714),
        glm::vec3( 8.653,  -4.582,   8.562),
        glm::vec3( 7.427,  -1.567,   6.596),
        glm::vec3( 6.631,  -3.764,   3.624),
        glm::vec3( 4.949,  -6.246,   6.024),
        glm::vec3( 2.680,  -3.622,   7.319),
        glm::vec3( 1.661,  -2.534,   3.797),
        glm::vec3( 1.047,  -6.177,   2.834),
        glm::vec3(-1.295,  -6.482,   5.820),
        glm::vec3(-3.420,  -3.432,   4.910),
        glm::vec3(-5.804,  -5.385,   2.594),
    };
    std::vector<glm::vec3> orientationVectors = {
        //glm::vec3(0.0f, -1.0f, 0.0f),
        //glm::vec3(0.0f, -1.0f, 0.0f),
        //glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f)),
        //glm::vec3(1.0f, 0.0f, 0.0f),
        //glm::vec3(0.0f, 1.0f, 0.0f),
        //glm::normalize(glm::vec3(-1.0f, 1.0f, 0.0f)),
        //glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)),
        //glm::vec3(0.0f, -1.0f, 0.0f),
        //glm::vec3(0.0f, -1.0f, 0.0f),

        glm::normalize(glm::vec3( 5.379, -13.834,  15.145) - glm::vec3( 4.047, -12.839,  16.901)),
        glm::normalize(glm::vec3( 7.152,  -9.941,  15.050) - glm::vec3( 5.757, -11.521,  13.850)),
        glm::normalize(glm::vec3( 9.868, -11.242,  11.386) - glm::vec3( 9.465, -10.668,  13.672)),
        glm::normalize(glm::vec3(13.302,  -8.278,  11.761) - glm::vec3(11.204,  -8.935,  10.767)),
        glm::normalize(glm::vec3(16.635,  -7.542,   9.793) - glm::vec3(14.831,  -9.075,   9.617)),
        glm::normalize(glm::vec3(12.760,  -4.866,   8.857) - glm::vec3(15.122,  -5.348,   8.865)),
        glm::normalize(glm::vec3(11.077,  -2.142,   7.663) - glm::vec3(13.127,  -2.024,   8.869)),
        glm::normalize(glm::vec3(10.150,  -3.168,   4.873) - glm::vec3(12.305,  -2.325,   5.169)),
        glm::normalize(glm::vec3( 8.570,  -5.895,   6.136) - glm::vec3(10.923,  -5.730,   5.714)),
        glm::normalize(glm::vec3( 6.496,  -3.756,   7.879) - glm::vec3( 8.653,  -4.582,   8.562)),
        glm::normalize(glm::vec3( 5.448,  -2.001,   5.264) - glm::vec3( 7.427,  -1.567,   6.596)),
        glm::normalize(glm::vec3( 4.524,  -4.862,   3.627) - glm::vec3( 6.631,  -3.764,   3.624)),
        glm::normalize(glm::vec3( 2.554,  -6.105,   6.047) - glm::vec3( 4.949,  -6.246,   6.024)),
        glm::normalize(glm::vec3( 0.594,  -3.147,   6.222) - glm::vec3( 2.680,  -3.622,   7.319)),
        glm::normalize(glm::vec3(-0.243,  -3.743,   2.943) - glm::vec3( 1.661,  -2.534,   3.797)),
        glm::normalize(glm::vec3(-1.190,  -7.047,   3.135) - glm::vec3( 1.047,  -6.177,   2.834)),
        glm::normalize(glm::vec3(-3.614,  -5.945,   5.977) - glm::vec3(-1.295,  -6.482,   5.820)),
        glm::normalize(glm::vec3(-5.776,  -3.791,   4.790) - glm::vec3(-3.420,  -3.432,   4.910)),
        glm::normalize(glm::vec3(-7.369,  -7.123,   2.981) - glm::vec3(-5.804,  -5.385,   2.594)),
    };

    // Center points
    glm::vec3 center;
    for (auto p : controlPoints) {
        center += p;
    }
    center /= controlPoints.size();
    for (auto &p : controlPoints) {
        p -= center;
        // NOTE Manual adjustment
        p.x -= 2.0f;
        p.y -= 2.0f;
        p.z += 2.0f;
    }

    // Generate orientation vectors
    //for (int i = 1; i < controlPoints.size() - 1; i++) {
    //    glm::vec3 a = controlPoints[i - 1];
    //    glm::vec3 b = controlPoints[i];
    //    glm::vec3 c = controlPoints[i + 1];
    //    glm::vec3 n = glm::normalize((b - c) + (b - a));
    //    orientationVectors.push_back(n);
    //}
    //orientationVectors.insert(orientationVectors.begin(), orientationVectors[0]);
    //orientationVectors.push_back(orientationVectors.back());

    BSpline spline(controlPoints, orientationVectors, 3);
    auto curvePoints = spline.generateCurve(100);
    //for (glm::vec3 p : curvePoints) {
    //    std::cout << p.x << ", " << p.y << ", " << p.z << ")\n";
    //}

    // Create Objects
    DrawObject mesh = createTubeMesh(spline, 200, 12, 1.0f);
    DrawObject spheres = createSpheres(controlPoints);
    DrawObject cylinders = createCylinders(curvePoints);

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
        glShadeModel(GL_SMOOTH);
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
