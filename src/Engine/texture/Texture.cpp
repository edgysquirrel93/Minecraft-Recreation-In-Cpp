#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace engine::texture {

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
    // ui
    stbi_set_flip_vertically_on_load(false);
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

    // In game textures
    stbi_set_flip_vertically_on_load(true);
    block.dirt.loadTexture("assets/Textures/Blocks/dirt.png");
}

} // engine::texture
