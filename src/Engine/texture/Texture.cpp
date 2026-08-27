#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>

namespace engine::texture {

void LoadTexture::loadAtlas() {
    constexpr int tileSize = 16;
    constexpr int atlasColumns = 4;
    constexpr int atlasRows = 4;
    constexpr int atlasWidth = tileSize * atlasColumns;
    constexpr int atlasHeight = tileSize * atlasRows;
    constexpr int channels = 4;

    std::vector<unsigned char> atlasData(atlasWidth * atlasHeight * channels, 0);

    // Block list and coords x, y
    struct TileInfo {
        std::string path;
        int col;
        int row;
    };

    const std::vector<TileInfo> blocks = {
        {.path = "assets/Textures/Blocks/dirt.png", .col = 0, .row = 0},
        {.path = "assets/Textures/Blocks/stone.png", .col = 1, .row = 0}
    };

    for (const auto& [path, col, row] : blocks) {
        int w, h, comp;

        if (unsigned char* imgData = stbi_load(path.c_str(), &w, &h, &comp, 4)) {
            if (w == tileSize && h == tileSize) {
                for (int y = 0; y < tileSize; ++y) {
                    for (int x = 0; x < tileSize; ++x) {
                        const int atlasX = (col * tileSize) + x;
                        const int atlasY = (row * tileSize) + y;

                        const int atlasIndex = (atlasY * atlasWidth + atlasX) * channels;
                        const int imgIndex = (y * tileSize + x) * channels;

                        atlasData[atlasIndex + 0] = imgData[imgIndex + 0]; // R
                        atlasData[atlasIndex + 1] = imgData[imgIndex + 1]; // G
                        atlasData[atlasIndex + 2] = imgData[imgIndex + 2]; // B
                        atlasData[atlasIndex + 3] = imgData[imgIndex + 3]; // A
                    }
                }
            }
            stbi_image_free(imgData);
        } else {
            std::cout << "Failed to load atlas tile: " << path << std::endl;
        }
    }

    unsigned int atlasTextureID;
    glGenTextures(1, &atlasTextureID);
    glBindTexture(GL_TEXTURE_2D, atlasTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    block.atlas.setId(atlasTextureID);
}

void Texture::loadTexture(const std::string& path, const GLenum filtering) {
    int width, height, nrComponents;

    if (unsigned char* data {stbi_load(path.c_str(), &width, &height, &nrComponents, 0)}) {
        glGenTextures(1, &m_Id);
        glBindTexture(GL_TEXTURE_2D, m_Id);

        const GLenum format {static_cast<GLenum>(nrComponents == 4 ? GL_RGBA : GL_RGB)};

        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height,
            0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filtering));

        const GLenum minFilter {static_cast<GLenum>(filtering == GL_NEAREST ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_LINEAR)};
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        m_Id = 0;
    }
}
void LoadTexture::loadAllTextures() {
    stbi_set_flip_vertically_on_load(false);
    // Blocks
    loadAtlas();

    // ui
    ui.logo.loadTexture("assets/Textures/UI/minecraft_logo.png");
    ui.slider.loadTexture("assets/Textures/UI/slider.png");
    ui.slider_handle.loadTexture("assets/Textures/UI/slider_handle.png");
    ui.slider_handle_highlighted.loadTexture("assets/Textures/UI/slider_handle_highlighted.png");
    ui.slider_highlighted.loadTexture("assets/Textures/UI/slider_highlighted.png");
    ui.button.loadTexture("assets/Textures/UI/button.png");
    ui.button_highlighted.loadTexture("assets/Textures/UI/button_highlighted.png");
    ui.button_disabled.loadTexture("assets/Textures/UI/button_disabled.png");
    ui.text_field.loadTexture("assets/Textures/UI/text_field.png");
    ui.text_field_highlighted.loadTexture("assets/Textures/UI/text_field_highlighted.png");
    ui.scroller.loadTexture("assets/Textures/UI/scroller.png");
    ui.scroller_background.loadTexture("assets/Textures/UI/scroller_background.png");
    ui.dirt_ui.loadTexture("assets/Textures/Blocks/dirt.png");
}

} // engine::texture
