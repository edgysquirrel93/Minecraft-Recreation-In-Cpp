cmake_minimum_required(VERSION 3.18)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
project(Minecraft_Recreation_Recreation)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets")
file(MAKE_DIRECTORY "${ASSET_DIR}")

set(FONTS_URL "https://github.com/IdreesInc/Monocraft/releases/download/v4.2.1/Monocraft-ttf.zip")
set(SONGS_URL "https://github.com/edgysquirrel93/Minecraft-Recreation-In-Cpp/releases/download/assets-songs/assets.zip")
set(SHADERS_URL "https://github.com/edgysquirrel93/Minecraft-Recreation-In-Cpp/releases/download/assets-shaders/assets.zip")
set(TEXTURES_URL "https://github.com/edgysquirrel93/Minecraft-Recreation-In-Cpp/releases/download/assets-textures/assets.zip")
set(SOUNDEFFECTS_URL "https://github.com/edgysquirrel93/Minecraft-Recreation-In-Cpp/releases/download/assets-soundeffects/assets.zip")

set(FONTS_CHECK_DIR "${ASSET_DIR}/Fonts/Monocraft-ttf")
set(SONGS_CHECK_DIR "${ASSET_DIR}/Sound/Music")
set(SOUNDEFFECTS_CHECK_DIR "${ASSET_DIR}/Sound/SoundEffects")
set(SHADERS_CHECK_DIR "${ASSET_DIR}/Shaders/Vertices")
set(TEXTURES_CHECK_DIR "${ASSET_DIR}/Textures/Blocks")

macro(download_and_extract url zip_name check_dir extract_dest asset_type)
    if(NOT EXISTS "${check_dir}")
        message(STATUS "${asset_type} not found! Downloading from ${url}...")
        file(DOWNLOAD "${url}" "${CMAKE_BINARY_DIR}/${zip_name}" SHOW_PROGRESS STATUS download_status)
        list(GET download_status 0 status_code)
        list(GET download_status 1 status_message)

        if(status_code EQUAL 0)
            message(STATUS "Download complete! Extracting ${asset_type}...")
            file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/${zip_name}" DESTINATION "${extract_dest}")
            file(REMOVE "${CMAKE_BINARY_DIR}/${zip_name}")
        else()
            message(FATAL_ERROR "Failed to download ${asset_type}: ${status_message}")
        endif()
    endif()
endmacro()

download_and_extract("${FONTS_URL}" "temp_fonts.zip" "${FONTS_CHECK_DIR}" "${ASSET_DIR}/Fonts/" "Fonts")
download_and_extract("${SONGS_URL}" "temp_songs.zip" "${SONGS_CHECK_DIR}" "${ASSET_DIR}/Sound/Music" "Songs")
download_and_extract("${SOUNDEFFECTS_URL}" "temp_soundeffects.zip" "${SOUNDEFFECTS_CHECK_DIR}" "${ASSET_DIR}/Sound/SoundEffects" "Sound Effects")
download_and_extract("${SHADERS_URL}" "temp_shaders.zip" "${SHADERS_CHECK_DIR}" "${ASSET_DIR}/Shaders" "Shaders")
download_and_extract("${TEXTURES_URL}" "temp_textures.zip" "${TEXTURES_CHECK_DIR}" "${ASSET_DIR}/Textures" "Textures")

include(FetchContent)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(glfw)

# GLAD
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/FelixHommel/Glad2-CMake.git GIT_TAG main GIT_SHALLOW ON)
FetchContent_MakeAvailable(glad)
add_library(my_glad STATIC ${glad_SOURCE_DIR}/src/gl.c)
target_include_directories(my_glad PUBLIC ${glad_SOURCE_DIR}/include)

# ImGui
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(imgui)

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
target_link_libraries(imgui_lib PRIVATE glfw my_glad)

# GLM
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 0.9.9.8 GIT_SHALLOW ON)
FetchContent_MakeAvailable(glm)

# STB
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master GIT_SHALLOW ON)
FetchContent_MakeAvailable(stb)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# nlohmann/json
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3 GIT_SHALLOW ON)
FetchContent_MakeAvailable(json)

# Miniaudio
FetchContent_Declare(mini_audio GIT_REPOSITORY https://github.com/mackron/miniaudio GIT_TAG master GIT_SHALLOW ON)
FetchContent_MakeAvailable(mini_audio)

# Perlin Noise
FetchContent_Declare(perlin_noise URL https://raw.githubusercontent.com/Reputeless/PerlinNoise/bdf39fe92b2a585cdef485bcec2bca8ab5614095/PerlinNoise.hpp DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(perlin_noise)
add_library(perlin_noise INTERFACE)
target_include_directories(perlin_noise INTERFACE ${perlin_noise_SOURCE_DIR})

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
        src/Engine/block/Block.h
        src/Engine/block/Block.cpp
        src/Engine/rendering/ChunkRendering.cpp
        src/Engine/rendering/ChunkRendering.h
        src/Engine/world/World.cpp
        src/Engine/world/World.h
)
target_include_directories(Minecraft_Recreation_Recreation PRIVATE
        src
        ${glfw_SOURCE_DIR}/include
        ${glad_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${mini_audio_SOURCE_DIR}
)

find_package(Threads REQUIRED)

target_link_libraries(Minecraft_Recreation_Recreation PRIVATE
        glfw
        my_glad
        imgui_lib
        glm::glm
        nlohmann_json::nlohmann_json
        stb_image
        perlin_noise
        Threads::Threads
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Minecraft_Recreation_Recreation PRIVATE
            asound
            pulse
            dl
            m
    )
endif()