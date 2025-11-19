#include "gl.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glm/ext/scalar_constants.hpp>
#include "window.h"
#include <iostream>

// Set GL version
const char* glsl_version = "#version 330";

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
    int lod = 0;
};

void settingsUI(Settings &settings) {
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    // Settings that apply to all objects
    ImGui::Text("Draw settings");
    ImGui::Indent();
    {
        ImGui::Checkbox("Draw wireframes", &settings.drawWireframes);
        ImGui::Checkbox("Draw normals", &settings.uniforms.drawNormals);
        if (settings.uniforms.drawNormals) ImGui::BeginDisabled();
        {
            ImGui::SetNextItemWidth(128);
            ImGui::SliderFloat("Light", &settings.uniforms.lightIntensity, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::SetNextItemWidth(128);
            ImGui::SliderFloat("Ambient", &settings.uniforms.ambientLightIntensity, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        }
        if (settings.uniforms.drawNormals) ImGui::EndDisabled();
    }
    ImGui::Unindent();

    ImGui::Spacing();

    // Spheres parameters
    ImGui::Checkbox("Sphere impostors", &settings.drawSpheres);
    ImGui::Indent();
    {
        if (!settings.drawSpheres) ImGui::BeginDisabled();
        {
            ImGui::SetNextItemWidth(128);
            ImGui::SliderFloat("Radius##Sphere", &settings.uniforms.sphereRadius, 0.1f, 2.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::Checkbox("Ray traced", &settings.uniforms.raytraced);
        }
        if (!settings.drawSpheres) ImGui::EndDisabled();
    }
    ImGui::Unindent();

    ImGui::Spacing();

    // Cylinders parameters
    ImGui::Checkbox("Cylinder impostors", &settings.drawCylinders);
    ImGui::Indent();
    {
        if (!settings.drawCylinders) ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(128);
        ImGui::SliderFloat("Radius##Cylinder", &settings.uniforms.cylinderRadius, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SetNextItemWidth(128);
        ImGui::Combo("Mode", &settings.uniforms.cylinderMode, "Simple\0Rounded\0Ribbon\0");
        if (settings.uniforms.cylinderMode != 2) ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(128);
        ImGui::SliderFloat("Pitch", &settings.uniforms.pitch, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SetNextItemWidth(128);
        ImGui::SliderFloat("Width", &settings.uniforms.width, 0.05f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
        if (settings.uniforms.cylinderMode != 2) ImGui::EndDisabled();
        if (!settings.drawCylinders) ImGui::EndDisabled();
    }
    ImGui::Unindent();

    ImGui::Spacing();

    // Mesh settings
    ImGui::Checkbox("Mesh", &settings.drawMesh);
    ImGui::Indent();
    if (!settings.drawMesh) ImGui::BeginDisabled();
    {
        ImGui::SetNextItemWidth(128);
        const char *lods[] = { "Auto", "0", "1", "2" };
        ImGui::Combo("LOD", &settings.lod, lods, 4);
        ImGui::SetNextItemWidth(128);
        const char *textureModes[] = { "Off", "On", "Texture only" };
        ImGui::Combo("Texture", &settings.uniforms.drawTexture, textureModes, 3);
        ImGui::Checkbox("Checkerboard", &settings.uniforms.checkerboard);
    }
    if (!settings.drawMesh) ImGui::EndDisabled();
    ImGui::Unindent();

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

    // Set default camera and display settings
    Camera camera;
    Settings settings;

    // Create input struct and let GLFW access it
    MouseState mouse;
    glfwSetWindowUserPointer(window, &mouse);

    // Create spline
    BSpline spline = exampleSpline(false);
    std::vector<glm::vec3> controlPoints = spline.getControlPoints();

    // Create ball-and-stick objects
    Spheres spheres = createSpheres(controlPoints);
    Cylinders cylinders = createCylinders(controlPoints);
    //auto curvePoints = spline.generateCurve(nSegments);
    //Cylinders cylinders = createCylinders(curvePoints);

    // Create spline mesh at several levels of detail
    std::cout << "Building meshes..." << std::endl;
    unsigned int nSegments = (unsigned int)(spline.arcLength(1.0f)); // One segment per unit of distance at lowest LOD
    // NOTE Because vertices are reused, wireframe indices are only correct when loopResolution is 4 / 10 / 16
    Mesh lod0 = createSplineMesh(spline, nSegments * 4, 16, 1.0f);
    std::cout << "LOD 0: " << lod0.vertices.size() << " vertices" << std::endl;
    Mesh lod1 = createSplineMesh(spline, nSegments * 2, 10, 1.0f);
    std::cout << "LOD 1: " << lod1.vertices.size() << " vertices" << std::endl;
    Mesh lod2 = createSplineMesh(spline, nSegments * 1, 4, 1.0f);
    std::cout << "LOD 2: " << lod2.vertices.size() << " vertices" << std::endl;

    // Bake lightmap
    std::cout << "Baking lightmap..." << std::endl;
    GLuint lightmap = 0;
    bakeLightmap(&lightmap, lod1, shaders.meshProgram); // Use LOD 1 as tradeoff between quality and generation speed

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
        glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);

        // Select level of detail
        DrawObject *mesh;
        switch (settings.lod) {
            case 0:
                if (camera.dist > 200.0f) { mesh = &lod2; }
                else if (camera.dist > 50.0f) { mesh = &lod1; }
                else { mesh = &lod0; }
                break;
            case 1: mesh = &lod0; break;
            case 2: mesh = &lod1; break;
            case 3: mesh = &lod2; break;
        }

        // Bind lightmap texture
        // TODO Move into mesh?
        GLint uniformLoc = glGetUniformLocation(shaders.meshProgram, "lightmap");
        glUniform1i(uniformLoc, 0);
        glBindTexture(GL_TEXTURE_2D, lightmap);

        // Draw objects
        glShadeModel(GL_SMOOTH);
        glEnable(GL_CULL_FACE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glDepthRange(0.0, 1.0);
        glPolygonOffset(0.0, 0.0);
        if (settings.drawMesh) draw(*mesh, shaders.meshProgram, settings.uniforms);
        if (settings.drawSpheres) draw(spheres, shaders.sphereProgram, settings.uniforms);
        if (settings.drawCylinders) draw(cylinders, shaders.cylinderProgram, settings.uniforms);
        if (settings.drawWireframes) {
            // Draw wireframes with no backface culling and on top of everything else
            // TODO Make this a setting
            //glDisable(GL_CULL_FACE);
            //glDepthRange(0.0, 0.0);
            glPolygonOffset(-1.0, 0.0);
            if (settings.drawMesh) draw(*mesh, shaders.meshWireframeProgram, settings.uniforms);
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
    glDeleteTextures(1, &lightmap);
    destroyWindow(window);

    return 0;
}
