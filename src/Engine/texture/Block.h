#ifndef MINECRAFT_RECREATION_RECREATION_BLOCK_H
#define MINECRAFT_RECREATION_RECREATION_BLOCK_H
#include <array>

#include "Texture.h"

struct BlockType {
    std::string name {"Air"};
    std::array<int, 6> faceLayers {-1, -1, -1, -1, -1, -1}; // Back, Front, Left, Right, Bottom, Top

    BlockType() = default;

    // Single faced block
    BlockType(std::string blockName, const int singleLayer)
        : name(std::move(blockName)),
          faceLayers{singleLayer, singleLayer, singleLayer, singleLayer, singleLayer, singleLayer} {}

    // Top, Bottom, and Sides
    BlockType(std::string blockName, const int top, const int bottom, const int side)
        : name(std::move(blockName)),
          faceLayers{side, side, side, side, bottom, top} {}

    // 6 faces
    BlockType(std::string blockName, const std::array<int, 6>& layers)
        : name(std::move(blockName)),
          faceLayers(layers) {}

    bool operator==(const BlockType& other) const {return name == other.name;}
    bool operator!=(const BlockType& other) const {return !(*this == other);}
};

namespace engine::blockregistry {
    using namespace engine::texture;

    constexpr uint8_t ID_AIR     = 0;
    constexpr uint8_t ID_DIRT    = 1;
    constexpr uint8_t ID_STONE   = 2;
    constexpr uint8_t ID_GRASS   = 3;
    constexpr uint8_t ID_BEDROCK = 4;

    inline const BlockType AIR{"Air", -1};

    inline const BlockType DIRT{"Dirt", BlockLayer::DIRT};

    inline const BlockType STONE{"Stone", BlockLayer::STONE};

    inline const BlockType GRASS{
        "Grass",
        BlockLayer::GRASS_TOP, BlockLayer::DIRT, BlockLayer::GRASS_SIDE
    };

    inline const BlockType BEDROCK{"Bedrock", BlockLayer::BEDROCK};

    inline const std::array REGISTRY = {
        AIR,      // ID 0
        DIRT,     // ID 1
        STONE,    // ID 2
        GRASS,    // ID 3
        BEDROCK   // ID 4
    };

    [[nodiscard]] inline const BlockType& get(const uint8_t id) {
        if (id >= REGISTRY.size()) return AIR;
        return REGISTRY[id];
    }
}
#endif