#include "WorldGen.h"

#include <ranges>

#include "Engine/config/SettingsManager.h"


namespace engine::worldgen {

const BlockType& World::getBlockAt(const int worldX, const int worldY, const int worldZ) const {
    if (worldY < 0 || worldY >= 256) return blockregistry::get(blockregistry::ID_AIR);

    const int chunkX{toChunkCoord(worldX)};
    const int chunkZ{toChunkCoord(worldZ)};

    if (const ChunkRendering* chunk{getChunk(chunkX, chunkZ)}) {
        return chunk->getBlockAt(toLocalCoord(worldX), worldY, toLocalCoord(worldZ));
    }
    return blockregistry::get(blockregistry::ID_AIR);
}

void World::setBlockAt(const int worldX, const int worldY, const int worldZ, const uint16_t blockID) {
    if (worldY < 0 || worldY >= 256) return;

    const int chunkX{toChunkCoord(worldX)};
    const int chunkZ{toChunkCoord(worldZ)};
    const int localX{toLocalCoord(worldX)};
    const int localZ{toLocalCoord(worldZ)};

    if (ChunkRendering* chunk = getChunk(chunkX, chunkZ)) {
        chunk->setBlock(localX, worldY, localZ, blockID);
        chunk->makeDirty();

        if (localX == 0) {
            if (auto* neighbor = getChunk(chunkX - 1, chunkZ)) neighbor->makeDirty();
        } else if (localX == 15) {
            if (auto* neighbor = getChunk(chunkX + 1, chunkZ)) neighbor->makeDirty();
        }

        if (localZ == 0) {
            if (auto* neighbor = getChunk(chunkX, chunkZ - 1)) neighbor->makeDirty();
        } else if (localZ == 15) {
            if (auto* neighbor = getChunk(chunkX, chunkZ + 1)) neighbor->makeDirty();
        }
    }
}

void World::update(const glm::vec3& playerPos) {
    const int renderDistance = config::SettingsManager::get().getRenderDistance();

    const int centerChunkX{toChunkCoord(static_cast<int>(playerPos.x))};
    const int centerChunkZ{toChunkCoord(static_cast<int>(playerPos.z))};

    for (int x = centerChunkX - renderDistance; x <= centerChunkX + renderDistance; ++x) {
        for (int z = centerChunkZ - renderDistance; z <= centerChunkZ + renderDistance; ++z) {
            if (const uint64_t key = getChunkKey(x, z); !m_Chunks.contains(key)) {
                auto chunk = std::make_unique<ChunkRendering>(x, z);
                chunk->generateChunk();
                m_Chunks[key] = std::move(chunk);
            }
        }
    }

    for (const auto& chunk : m_Chunks | std::views::values) {
        if (chunk->isDirty()) {
            chunk->rebuildMesh(*this);
        }
    }

    for (auto it = m_Chunks.begin(); it != m_Chunks.end();) {
        if (std::abs(it->second->getChunkX() - centerChunkX) > renderDistance + 1 ||
            std::abs(it->second->getChunkZ() - centerChunkZ) > renderDistance + 1) {
            it = m_Chunks.erase(it);
            } else {
                ++it;
            }
    }
}
} // engine::worldgen