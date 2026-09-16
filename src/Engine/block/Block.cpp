#include "Block.h"

namespace engine::blockregistry {
using namespace engine::texture;

Block& Block::instance() {
    static Block registryInstance;
    return registryInstance;
}

Block::Block()
    : m_Registry{
{"Air",     -1, false},
{"Dirt",    BlockLayer::DIRT},
{"Stone",   BlockLayer::STONE},
{"Grass",   BlockLayer::GRASS_TOP, BlockLayer::DIRT, BlockLayer::GRASS_SIDE},
{"Bedrock", BlockLayer::BEDROCK},
{"Glass", BlockLayer::GLASS, false},
{"Water", BlockLayer::WATER, false},
{"Sand", BlockLayer::SAND},
{"Oak_Log", BlockLayer::OAK_LOG_TOP, BlockLayer::OAK_LOG_TOP, BlockLayer::OAK_LOG},
{"Oak_Planks", BlockLayer::OAK_PLANKS},
{"Oak_Leaves", BlockLayer::OAK_LEAVES, false}
    } {}

const block::BlockType& Block::getBlock(const uint8_t id) const {
    if (id >= m_Registry.size()) {
        return m_Registry[ID_AIR];
    }
    return m_Registry[id];
}

const block::BlockType& get(const uint8_t id) {return Block::instance().getBlock(id);}

}
