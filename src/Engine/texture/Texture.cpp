#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>

namespace engine::texture {

void LoadTexture::loadTextureArray() {
    constexpr int tileSize {16};

    struct BlockInfo {
        std::string path;
        int layer;
    };

    const std::vector<BlockInfo> blocks = {
        {.path = "assets/Textures/Blocks/dirt.png",       .layer = BlockLayer::DIRT},
        {.path = "assets/Textures/Blocks/stone.png",      .layer = BlockLayer::STONE},
        {.path = "assets/Textures/Blocks/grass_block_top.png",  .layer = BlockLayer::GRASS_TOP},
        {.path = "assets/Textures/Blocks/grass_block_side.png", .layer = BlockLayer::GRASS_SIDE}
    };

    const int totalLayers {static_cast<int>(blocks.size())};

    unsigned int textureArrayID;
    glGenTextures(1, &textureArrayID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, tileSize, tileSize, totalLayers);

    for (const auto& [path, layer] : blocks) {
        int w, h, comp;

        if (unsigned char* imgData = stbi_load(path.c_str(), &w, &h, &comp, 4)) {
            if (w == tileSize && h == tileSize) {
                glTexSubImage3D(
                    GL_TEXTURE_2D_ARRAY,
                    0,                     // mipmap level
                    0, 0, layer,    // x, y, z
                    tileSize, tileSize, 1,  // Width, Height, Depth
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    imgData
                );
                std::cout << "Loading layer " << layer << ": " << path << " (" << w << "x" << h << ")\n";
            } else {
                std::cout << "Texture dimensions mismatch for: " << path << std::endl;
            }
            stbi_image_free(imgData);
        } else {
            std::cout << "Failed to load layer tile: " << path << std::endl;
        }
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    block.atlas.setId(textureArrayID);
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
    loadTextureArray();

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
