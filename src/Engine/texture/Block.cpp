#include "Block.h"

namespace engine::blockregistry {
using namespace engine::texture;

Block& Block::instance() {
    static Block registryInstance;
    return registryInstance;
}

    Block::Block()
        : m_Registry{
    {"Air",     -1},
    {"Dirt",    BlockLayer::DIRT},
    {"Stone",   BlockLayer::STONE},
    {"Grass",   BlockLayer::GRASS_TOP, BlockLayer::DIRT, BlockLayer::GRASS_SIDE},
    {"Bedrock", BlockLayer::BEDROCK}
        }
{}

const BlockType& Block::getBlock(const uint8_t id) const {
    if (id >= m_Registry.size()) {
        return m_Registry[ID_AIR];
    }
    return m_Registry[id];
}

const BlockType& get(const uint8_t id) {return Block::instance().getBlock(id);}

}
