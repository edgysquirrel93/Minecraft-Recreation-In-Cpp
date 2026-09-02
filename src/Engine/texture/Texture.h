#ifndef MINECRAFT_RECREATION_RECREATION_TEXTURE_H
#define MINECRAFT_RECREATION_RECREATION_TEXTURE_H
#include <glad/gl.h>
#include <iostream>

namespace engine::texture {

class Texture {
    unsigned int m_Id{};
public:
    void loadTexture(const std::string& path, GLenum filtering = GL_NEAREST);

    void setId(const unsigned int id) {m_Id = id;}

    explicit operator unsigned int() const {return m_Id;}
};

struct UITexture {
    Texture logo;
    Texture slider;
    Texture slider_handle;
    Texture slider_handle_highlighted;
    Texture slider_highlighted;
    Texture button;
    Texture button_highlighted;
    Texture button_disabled;
    Texture text_field;
    Texture text_field_highlighted;
    Texture scroller;
    Texture scroller_background;
    Texture dirt_ui;
};

struct BlockTexture {
     Texture atlas;
};

struct BlockLayer {
    static constexpr int DIRT       {0};
    static constexpr int STONE      {1};
    static constexpr int GRASS_TOP  {2};
    static constexpr int GRASS_SIDE {3};
    static constexpr int BEDROCK    {4};
    static constexpr int GLASS      {5};
};

class LoadTexture {
public:
    inline static UITexture ui;
    inline static BlockTexture block;

    static void loadTextureArray();
    static void loadAllTextures();
};

} // engine::texture

#endif
