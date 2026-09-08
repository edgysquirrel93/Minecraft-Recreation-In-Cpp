#include "World.h"

#include <fstream>

#include "Engine/config/SettingsManager.h"

namespace engine::world {

World::World()
    : m_WorldGen(config::LevelData::get().getSeed()) {}

void World::saveChunk(int cx, int cz, const rendering::ChunkRendering* chunk)
{
    if (!chunk->isModified()) return;

    std::string currentWorldName {config::LevelData::get().getCurrentWorldName()};

    if (currentWorldName.empty()) return;

    const std::filesystem::path worldDir {config::SettingsManager::getSaveDirectory() / "saves" / currentWorldName / "world"};
    std::filesystem::create_directories(worldDir);

    const auto targetFile {worldDir / std::format("chunk_{}_{}.dat", cx, cz)};
    const auto tempFile {worldDir / std::format("chunk_{}_{}.tmp", cx, cz)};

    bool writeSuccess {false};

    {
        std::ofstream out(tempFile, std::ios::binary);
        if (!out) return;

        ChunkHeader header{
            .magic = CHUNK_FILE_MAGIC,
            .version = CHUNK_FILE_VERSION,
            .dataSize = sizeof(chunk->getBlockIDs())
        };

        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(chunk->getBlockIDs().data()), sizeof(chunk->getBlockIDs()));
        writeSuccess = out.good();
    }

    if (writeSuccess) {
        std::filesystem::rename(tempFile, targetFile);
        const_cast<rendering::ChunkRendering*>(chunk)->clearModified();
    } else if (std::filesystem::exists(tempFile)) {
        std::filesystem::remove(tempFile);
    }
}
bool World::loadChunk(int cx, int cz, rendering::ChunkRendering* chunk) {

    std::string currentWorldName {config::LevelData::get().getCurrentWorldName()};

    if (currentWorldName.empty()) return false;

    const auto filename {config::SettingsManager::getSaveDirectory() / "saves" / currentWorldName / "world" /
        std::format("chunk_{}_{}.dat", cx, cz)};

    if (!std::filesystem::exists(filename)) return false;

    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;

    ChunkHeader header{};
    in.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != CHUNK_FILE_MAGIC ||
        header.version != CHUNK_FILE_VERSION ||
        header.dataSize != sizeof(chunk->getBlockIDs())) {
        return false;
        }

    in.read(reinterpret_cast<char*>(chunk->getBlockIDs().data()), sizeof(chunk->getBlockIDs()));

    if (in) {
        chunk->makeDirty();
        chunk->clearModified();
        return true;
    }

    return static_cast<bool>(in);
}

void World::saveAllChunks() {
    for (const auto& chunk : m_Chunks | std::views::values) {
        saveChunk(chunk->getChunkX(), chunk->getChunkZ(), chunk.get());
    }
}

const block::BlockType& World::getBlockAt(const int worldX, const int worldY, const int worldZ) const {
    if (worldY < 0 || worldY >= 256) return blockregistry::get(blockregistry::ID_AIR);

    const int chunkX{toChunkCoord(worldX)};
    const int chunkZ{toChunkCoord(worldZ)};

    if (const rendering::ChunkRendering* chunk{getChunk(chunkX, chunkZ)}) {
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

    if (rendering::ChunkRendering* chunk = getChunk(chunkX, chunkZ)) {
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
                auto chunk = std::make_unique<rendering::ChunkRendering>(x, z);
                if (!loadChunk(x, z, chunk.get())) {
                    m_WorldGen.generateChunkData(*chunk);
                }
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
            saveChunk(it->second->getChunkX(), it->second->getChunkZ(), it->second.get());
            it = m_Chunks.erase(it);
            } else {
                ++it;
        }
    }
}
void World::render() const {
    for (const auto& chunk : m_Chunks | std::views::values) {
        if (chunk && chunk->getVertex() > 0) {
            glBindVertexArray(chunk->getVAO());
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(chunk->getVertex()));
        }
    }
}
}
