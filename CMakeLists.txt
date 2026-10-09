cmake_minimum_required(VERSION 3.16)
project(myapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- GLFW ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

# --- Dear ImGui ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9
)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw imgui stb)

find_package(OpenGL REQUIRED)

# Build ImGui static library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# --- App Sources ---
set(APP_SOURCES
    src/main.cpp
    src/Wallpaper.cpp
    src/Taskbar.cpp
    src/StartMenu.cpp
    src/Apps/FileExplorer.cpp
    src/Apps/SystemInfo.cpp
    src/Apps/TaskManager.cpp
    src/Apps/SettingsApp.cpp
    src/Apps/WordleApp.cpp
    src/Apps/AiApp.cpp
    src/Apps/CalculatorApp.cpp
    src/Apps/ReelsApp.cpp
)

add_executable(myapp ${APP_SOURCES})
target_link_libraries(myapp PRIVATE imgui)
target_include_directories(myapp PRIVATE
    src
    ${stb_SOURCE_DIR}
)
target_compile_definitions(myapp PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}")