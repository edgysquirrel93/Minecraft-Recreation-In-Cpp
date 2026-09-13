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

const rendering::ChunkRendering* World::getChunk(const int chunkX, const int chunkZ) const {
    std::shared_lock lock(m_ChunksMutex);
    const uint64_t key = getChunkKey(chunkX, chunkZ);
    if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

rendering::ChunkRendering* World::getChunk(const int chunkX, const int chunkZ) {
    std::shared_lock lock(m_ChunksMutex);
    const uint64_t key = getChunkKey(chunkX, chunkZ);
    if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

void World::markNeighborsDirty(const int chunkX, const int chunkZ) {
    if (auto* north = getChunk(chunkX, chunkZ + 1)) north->makeDirty();
    if (auto* south = getChunk(chunkX, chunkZ - 1)) south->makeDirty();
    if (auto* east  = getChunk(chunkX + 1, chunkZ)) east->makeDirty();
    if (auto* west  = getChunk(chunkX - 1, chunkZ)) west->makeDirty();
}

uint8_t World::getBlockIDAt(const int worldX, const int worldY, const int worldZ) const {
    if (worldY < 0 || worldY >= 256) {
        return blockregistry::ID_AIR;
    }

    const int chunkX{toChunkCoord(worldX)};
    const int chunkZ{toChunkCoord(worldZ)};

    if (const rendering::ChunkRendering* chunk{getChunk(chunkX, chunkZ)}) {
        return chunk->getBlockID(toLocalCoord(worldX), worldY, toLocalCoord(worldZ));
    }

    return blockregistry::ID_AIR;
}

const block::BlockType& World::getBlockAt(const int worldX, const int worldY, const int worldZ) const {
    if (worldY < 0 || worldY >= 256) {
        return blockregistry::get(blockregistry::ID_AIR);
    }

    const int chunkX = toChunkCoord(worldX);
    const int chunkZ = toChunkCoord(worldZ);
    const uint64_t key = getChunkKey(chunkX, chunkZ);

    std::shared_lock lock(m_ChunksMutex);

    const auto it = m_Chunks.find(key);
    if (it == m_Chunks.end() || !it->second) {
        return blockregistry::get(blockregistry::ID_AIR);
    }

    const int localX = toLocalCoord(worldX);
    const int localZ = toLocalCoord(worldZ);

    return it->second->getBlockAt(localX, worldY, localZ);
}

void World::setBlockAt(const int worldX, const int worldY, const int worldZ, const uint16_t blockID) {
    if (worldY < 0 || worldY >= 256) return;

    const int chunkX{toChunkCoord(worldX)};
    const int chunkZ{toChunkCoord(worldZ)};
    const int localX{toLocalCoord(worldX)};
    const int localZ{toLocalCoord(worldZ)};

    if (rendering::ChunkRendering* chunk = getChunk(chunkX, chunkZ)) {
        chunk->setBlock(localX, worldY, localZ, blockID);

        const auto meshData = chunk->buildMeshDataCPU(*this);
        chunk->uploadGPU(meshData.vertices);

        if (localX == 0) {
            if (auto* neighbor = getChunk(chunkX - 1, chunkZ)) {
                neighbor->uploadGPU(neighbor->buildMeshDataCPU(*this).vertices);
            }
        } else if (localX == 15) {
            if (auto* neighbor = getChunk(chunkX + 1, chunkZ)) {
                neighbor->uploadGPU(neighbor->buildMeshDataCPU(*this).vertices);
            }
        }

        if (localZ == 0) {
            if (auto* neighbor = getChunk(chunkX, chunkZ - 1)) {
                neighbor->uploadGPU(neighbor->buildMeshDataCPU(*this).vertices);
            }
        } else if (localZ == 15) {
            if (auto* neighbor = getChunk(chunkX, chunkZ + 1)) {
                neighbor->uploadGPU(neighbor->buildMeshDataCPU(*this).vertices);
            }
        }
    }
}

void World::update(const glm::vec3& playerPos, util::ThreadPool& threadPool) {
    std::unique_ptr<rendering::ChunkRendering> generatedChunk;

    while (m_CompletedGenQueue.tryPop(generatedChunk)) {
        const int cx = generatedChunk->getChunkX();
        const int cz = generatedChunk->getChunkZ();
        const uint64_t key = getChunkKey(cx, cz);

        generatedChunk->makeDirty();

        {
            std::unique_lock lock(m_ChunksMutex);
            m_Chunks[key] = std::move(generatedChunk);
        }

        m_GeneratingChunkKeys.erase(key);

        markNeighborsDirty(cx, cz);
    }

    rendering::ChunkRendering::ChunkMeshData meshData;
    while (m_CompletedMeshQueue.tryPop(meshData)) {
        const uint64_t key = getChunkKey(meshData.chunkX, meshData.chunkZ);

        if (auto it = m_Chunks.find(key); it != m_Chunks.end()) {
            it->second->uploadGPU(meshData.vertices);
        }
        m_PendingMeshKeys.erase(key);
    }

    const int playerChunkX = toChunkCoord(static_cast<int>(std::floor(playerPos.x)));
    const int playerChunkZ = toChunkCoord(static_cast<int>(std::floor(playerPos.z)));
    const int renderDistance {config::SettingsManager::get().getRenderDistance()};

    for (int dx = -renderDistance; dx <= renderDistance; ++dx) {
        for (int dz = -renderDistance; dz <= renderDistance; ++dz) {
            const int cx = playerChunkX + dx;
            const int cz = playerChunkZ + dz;
            const uint64_t key = getChunkKey(cx, cz);

            if (m_Chunks.contains(key) || m_GeneratingChunkKeys.contains(key)) {
                continue;
            }

            m_GeneratingChunkKeys.insert(key);

            threadPool.enqueue([this, cx, cz]() {
                auto chunk = std::make_unique<rendering::ChunkRendering>(cx, cz);

                if (!loadChunk(cx, cz, chunk.get())) {
                    m_WorldGen.generateChunkData(*chunk);
                }

                m_CompletedGenQueue.push(std::move(chunk));
            });
        }
    }

    for (auto& [key, chunk] : m_Chunks) {
        if (chunk->isDirty() && !m_PendingMeshKeys.contains(key)) {
            m_PendingMeshKeys.insert(key);
            chunk->clearDirty();

            threadPool.enqueue([this, cX = chunk->getChunkX(), cZ = chunk->getChunkZ()]
            {
                if (const auto* targetChunk = getChunk(cX, cZ)) {
                    auto result = targetChunk->buildMeshDataCPU(*this);
                    m_CompletedMeshQueue.push(std::move(result));
                }
            });
        }
    }
}

void World::render(const shaders::Shader& shader) {
    shader.use();

    for (const auto& chunk : m_Chunks | std::views::values) {

        if (chunk->getVertex() == 0) continue;

        glBindVertexArray(chunk->getVAO());
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLint>(chunk->getVertex()));
    }

    glBindVertexArray(0);
}

}
