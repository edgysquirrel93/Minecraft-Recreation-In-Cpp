cmake_minimum_required(VERSION 4.2)
project(Mincraft_Recreation_Recreation)

set(CMAKE_CXX_STANDARD 20)

include_directories(Include)

add_subdirectory(Include/GLFW)

add_library(imgui STATIC
        Include/IMGUI/imgui.cpp
        Include/IMGUI/imgui_demo.cpp
        Include/IMGUI/imgui_draw.cpp
        Include/IMGUI/imgui_tables.cpp
        Include/IMGUI/imgui_widgets.cpp
        Include/IMGUI/backends/imgui_impl_opengl3.cpp
        Include/IMGUI/backends/imgui_impl_glfw.cpp
        Include/IMGUI/misc/cpp/imgui_stdlib.cpp
)

add_library(glad STATIC Include/GLAD/src/glad.c)

add_executable(Mincraft_Recreation_Recreation src/main.cpp
        src/Engine/ui/UIManager.cpp
        src/Engine/ui/UIManager.h
        src/Engine/window/WindowManager.h
        src/Engine/window/WindowManager.cpp
        src/Engine/texture/Texture.cpp
        src/Engine/texture/Texture.h
        src/Engine/sound/Sound.cpp
        src/Engine/sound/Sound.h
        src/Engine/config/SettingsManager.cpp
        src/Engine/config/SettingsManager.h
        src/Engine/engine/Engine.cpp
        src/Engine/engine/Engine.h)

target_link_libraries(Mincraft_Recreation_Recreation PUBLIC glfw)

target_link_libraries(Mincraft_Recreation_Recreation PRIVATE imgui glad)
