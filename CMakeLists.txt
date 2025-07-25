# Get the workspace folder name
get_filename_component(WORKSPACE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

cmake_minimum_required(VERSION 3.10)
project(BiosplinesViewer)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set preferred OpenGL library
set(OpenGL_GL_PREFERENCE GLVND)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

# Fetch imgui sources
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)

# Add ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
include_directories(${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

# Add executable with workspace folder name
add_executable(${WORKSPACE_NAME}
    ${IMGUI_SOURCES}
    src/gl.cpp
    src/window.cpp
    src/main.cpp
)

# Link libraries
target_link_libraries(${WORKSPACE_NAME} PRIVATE
    OpenGL::GL
    glfw
    GLEW::glew
)
