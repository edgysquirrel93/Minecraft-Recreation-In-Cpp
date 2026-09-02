cmake_minimum_required(VERSION 3.14)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
project(Minecraft_Recreation_Recreation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASSET_URL "https://github.com/edgysquirrel93/Minecraft-Recreation-In-Cpp/releases/download/assets-v0.1/assets.zip")
set(SHADERS_URL "https://github.com/edgysquirrel93/Minecraft-Recreation-In-Cpp/releases/download/assets-v0.2/assets.zip")
set(ASSET_ZIP_PATH "${CMAKE_BINARY_DIR}/assets.zip")
set(SHADERS_ZIP_PATH "${CMAKE_BINARY_DIR}/assets/Shaders.zip")
set(ASSET_EXTRACT_DIR "${CMAKE_BINARY_DIR}/assets")
set(SHADERS_EXTRACT_DIR "${CMAKE_BINARY_DIR}/assets/Shaders")

if(NOT EXISTS "${ASSET_EXTRACT_DIR}")
    message(STATUS "Assets not found! Downloading from ${ASSET_URL}...")

    file(DOWNLOAD "${ASSET_URL}" "${ASSET_ZIP_PATH}"
            SHOW_PROGRESS
            STATUS download_status
    )

    list(GET download_status 0 status_code)
    list(GET download_status 1 status_message)

    if(status_code EQUAL 0)
        message(STATUS "Download complete! Extracting to ${ASSET_EXTRACT_DIR}")

        file(ARCHIVE_EXTRACT INPUT "${ASSET_ZIP_PATH}" DESTINATION "${CMAKE_BINARY_DIR}")

        file(REMOVE "${ASSET_ZIP_PATH}")
    else()
        message(FATAL_ERROR "Failed to download assets: ${status_message}")
    endif()
endif()

if(NOT EXISTS "${SHADERS_EXTRACT_DIR}")
    message(STATUS "Shaders not found! Downloading from ${SHADERS_URL}...")

    file(DOWNLOAD "${SHADERS_URL}" "${SHADERS_ZIP_PATH}"
            SHOW_PROGRESS
            STATUS download_status
    )

    list(GET download_status 0 status_code)
    list(GET download_status 1 status_message)

    if(status_code EQUAL 0)
        message(STATUS "Download complete! Extracting to ${SHADERS_EXTRACT_DIR}")

        file(ARCHIVE_EXTRACT INPUT "${SHADERS_ZIP_PATH}" DESTINATION "${CMAKE_BINARY_DIR}")

        file(REMOVE "${SHADERS_ZIP_PATH}")
    else()
        message(FATAL_ERROR "Failed to download shaders: ${status_message}")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4 
        GIT_SHALLOW    ON
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/FelixHommel/Glad2-CMake.git
        GIT_TAG        main
        GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(glad)

add_library(my_glad STATIC
        ${glad_SOURCE_DIR}/src/gl.c
)
target_include_directories(my_glad PUBLIC
        ${glad_SOURCE_DIR}/include
)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.90.4 
        GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(imgui)

add_compile_definitions(GLFW_INCLUDE_NONE)

add_executable(Minecraft_Recreation_Recreation
        src/main.cpp
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
        src/Engine/engine/Engine.h
        src/Engine/shaders/shaders.h
        src/Engine/worldgen/WorldGen.cpp
        src/Engine/worldgen/WorldGen.h
        src/Engine/rendering/Rendering.cpp
        src/Engine/rendering/Rendering.h
        src/Engine/meshdata/MeshData.cpp
        src/Engine/meshdata/MeshData.h
        src/Engine/player/Input.cpp
        src/Engine/player/Input.h
        src/Engine/texture/Block.h
        src/Engine/worldgen/ChunkRendering.cpp
        src/Engine/worldgen/ChunkRendering.h
        src/Engine/texture/Block.cpp
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imgui_SOURCE_DIR}/misc/cpp
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        0.9.9.8
        GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
        GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(stb)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
        GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
        mini_audio
        GIT_REPOSITORY https://github.com/mackron/miniaudio
        GIT_TAG        master
        GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(mini_audio)

if(LINUX OR UNIX)
    target_link_libraries(Minecraft_Recreation_Recreation PRIVATE
            asound
            pulse
            pthread
            dl
            m
    )
endif()

target_link_libraries(imgui_lib PRIVATE glfw my_glad)

target_link_libraries(Minecraft_Recreation_Recreation PRIVATE
        glfw
        my_glad
        imgui_lib
        glm::glm
        nlohmann_json::nlohmann_json
        stb_image
)

target_include_directories(Minecraft_Recreation_Recreation PRIVATE
        src
        ${glfw_SOURCE_DIR}/include
        ${glad_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${mini_audio_SOURCE_DIR}
)