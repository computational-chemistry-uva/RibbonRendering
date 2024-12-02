# CMakeLists.txt
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

# Get the workspace folder name
get_filename_component(WORKSPACE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Add executable with workspace folder name
add_executable(${WORKSPACE_NAME} 
    $ENV{IMGUI_PATH}/imgui.cpp
    # $ENV{IMGUI_PATH}/imgui_stdlib.cpp
    $ENV{IMGUI_PATH}/imgui_draw.cpp
    $ENV{IMGUI_PATH}/imgui_tables.cpp
    $ENV{IMGUI_PATH}/imgui_widgets.cpp
    $ENV{IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    $ENV{IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    $ENV{IMGUI_PATH}/imgui_demo.cpp
    src/main.cpp
)

# Include directories
target_include_directories(${WORKSPACE_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

# Link SFML libraries
target_link_libraries(${WORKSPACE_NAME}
    OpenGL::GL
    glfw
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)
