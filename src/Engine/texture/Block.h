#ifndef MINECRAFT_RECREATION_RECREATION_BLOCK_H
#define MINECRAFT_RECREATION_RECREATION_BLOCK_H
#include <array>
#include <vector>

#include "Texture.h"

struct BlockType {
    std::string name {"Air"};
    std::array<int, 6> faceLayers {-1, -1, -1, -1, -1, -1}; // Back, Front, Left, Right, Bottom, Top
    bool isOpaque{true};

    BlockType() = default;

    // Single faced block
    BlockType(std::string blockName, const int singleLayer, const bool opaque = true)
        : name(std::move(blockName)),
          faceLayers{singleLayer, singleLayer, singleLayer, singleLayer, singleLayer, singleLayer},
          isOpaque(opaque) {}

    // Top, Bottom, and Sides
    BlockType(std::string blockName, const int top, const int bottom, const int side, const bool opaque = true)
        : name(std::move(blockName)),
          faceLayers{side, side, side, side, bottom, top} ,
          isOpaque(opaque) {}

    // 6 faces
    BlockType(std::string blockName, const std::array<int, 6>& layers, const bool opaque = true)
        : name(std::move(blockName)),
          faceLayers(layers),
          isOpaque(opaque) {}

    bool operator==(const BlockType& other) const {return name == other.name;}
    bool operator!=(const BlockType& other) const {return !(*this == other);}
};

namespace engine::blockregistry {

constexpr uint8_t ID_AIR     = 0;
constexpr uint8_t ID_DIRT    = 1;
constexpr uint8_t ID_STONE   = 2;
constexpr uint8_t ID_GRASS   = 3;
constexpr uint8_t ID_BEDROCK = 4;
constexpr uint8_t ID_GLASS   = 5;

class Block
{
    Block();
    std::vector<BlockType> m_Registry;
public:
    static Block& instance();

    Block(const Block&) = delete;
    Block& operator=(const Block&) = delete;

    [[nodiscard]] const BlockType& getBlock(uint8_t id) const;
};

[[nodiscard]] const BlockType& get(uint8_t id);

}
#endif