#ifndef MINECRAFT_RECREATION_RECREATION_BLOCK_H
#define MINECRAFT_RECREATION_RECREATION_BLOCK_H
#include <array>

#include "Texture.h"

struct BlockType {
    std::string name;
    std::array<int, 6> faceLayers; // Back, Front, Left, Right, Bottom, Top
};

namespace engine::blockregistry {
    using namespace engine::texture;

    inline const BlockType DIRT = {
        .name = "Dirt",
        .faceLayers = {
            BlockLayer::DIRT, BlockLayer::DIRT, BlockLayer::DIRT, BlockLayer::DIRT, BlockLayer::DIRT, BlockLayer::DIRT
        }
    };
    inline const BlockType STONE = {
        .name = "Stone",
        .faceLayers = {
            BlockLayer::STONE, BlockLayer::STONE, BlockLayer::STONE, BlockLayer::STONE, BlockLayer::STONE,
            BlockLayer::STONE
        }
    };
    inline const BlockType GRASS = {
        .name = "Grass",
        .faceLayers = {
            BlockLayer::GRASS_SIDE, BlockLayer::GRASS_SIDE, BlockLayer::GRASS_SIDE, BlockLayer::GRASS_SIDE,
            BlockLayer::DIRT, BlockLayer::GRASS_TOP
        }
    };
}
#endif