#include "World.h"

#include <fstream>

#include "Engine/config/SettingsManager.h"
#include "Engine/rendering/Rendering.h"
#include "Engine/ui/UIManager.h"

namespace engine::world {

World::World()
    : m_ThreadPool(std::max(1u, std::thread::hardware_concurrency() - 1)),
      m_WorldGen(config::LevelData::get().getSeed()) {}

NeighborChunks World::getNeighborSnapshot(const int cx, const int cz) const {
    std::shared_lock lock(m_ChunksMutex);
    return NeighborChunks{
        .center = getChunk(cx, cz),
        .north  = getChunk(cx, cz + 1),
        .south  = getChunk(cx, cz - 1),
        .east   = getChunk(cx + 1, cz),
        .west   = getChunk(cx - 1, cz)
    };
}

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
    return getChunkUnlocked(chunkX, chunkZ);
}

rendering::ChunkRendering* World::getChunk(const int chunkX, const int chunkZ) {
    std::shared_lock lock(m_ChunksMutex);
    return getChunkUnlocked(chunkX, chunkZ);
}

const rendering::ChunkRendering* World::getChunk(const int chunkX, const int chunkY, const int chunkZ) const {
    std::shared_lock lock(m_ChunksMutex);
    return getChunkUnlocked(chunkX, chunkY, chunkZ);
}

rendering::ChunkRendering* World::getChunk(const int chunkX, const int chunkY, const int chunkZ) {
    std::shared_lock lock(m_ChunksMutex);
    return getChunkUnlocked(chunkX, chunkY, chunkZ);
}

const rendering::ChunkRendering* World::getChunkUnlocked(const int chunkX, const int chunkZ) const {
    const uint64_t key = getChunkKey(chunkX, chunkZ);
    if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}
rendering::ChunkRendering* World::getChunkUnlocked(const int chunkX, const int chunkZ) {
    const uint64_t key = getChunkKey(chunkX, chunkZ);
    if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}
const rendering::ChunkRendering* World::getChunkUnlocked(const int chunkX, const int chunkY, const int chunkZ) const {
    const uint64_t key = getChunkKey(chunkX, chunkY, chunkZ);
    if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}
rendering::ChunkRendering* World::getChunkUnlocked(const int chunkX, const int chunkY, const int chunkZ) {
    const uint64_t key = getChunkKey(chunkX, chunkY, chunkZ);
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

    const int cx = toChunkCoord(worldX);
    const int cz = toChunkCoord(worldZ);
    const int lx = toLocalCoord(worldX);
    const int ly = worldY;
    const int lz = toLocalCoord(worldZ);
    const int subY = ly / 16;

    rendering::ChunkRendering* chunk = getChunk(cx, cz);
    if (!chunk) return;

    chunk->setBlock(lx, ly, lz, blockID);

    auto remeshSubChunk = [this](const int chunkX, const int chunkZ, const int sY) {
        if (sY < 0 || sY >= 16) return;
        if (auto* targetChunk = getChunk(chunkX, chunkZ)) {
            const NeighborChunks snapshot = getNeighborSnapshot(chunkX, chunkZ);
            const auto mesh = targetChunk->buildSectionMeshDataCPU(snapshot, sY);
            targetChunk->uploadSectionGPU(sY, mesh.vertices);
        }
    };

    remeshSubChunk(cx, cz, subY);

    const int localY = ly % 16;
    if (localY == 0)  remeshSubChunk(cx, cz, subY - 1);
    if (localY == 15) remeshSubChunk(cx, cz, subY + 1);

    if (lx == 0)  remeshSubChunk(cx - 1, cz, subY);
    if (lx == 15) remeshSubChunk(cx + 1, cz, subY);
    if (lz == 0)  remeshSubChunk(cx, cz - 1, subY);
    if (lz == 15) remeshSubChunk(cx, cz + 1, subY);
}

void World::setBlockWorldGen(const int worldX, const int worldY, const int worldZ, const uint16_t blockID) {
    if (worldY < 0 || worldY >= 256) return;

    const int cx = toChunkCoord(worldX);
    const int cz = toChunkCoord(worldZ);
    const int lx = toLocalCoord(worldX);
    const int lz = toLocalCoord(worldZ);

    if (rendering::ChunkRendering* chunk = getChunk(cx, cz)) {
        chunk->setBlock(lx, worldY, lz, blockID);
        chunk->makeDirty();
    }
}

void World::update(const glm::vec3& playerPos) {

    const auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
    m_lastFrameTime = currentTime;

    deltaTime = std::min(deltaTime, 0.1f);

    std::unique_ptr<rendering::ChunkRendering> generatedChunk;
    while (m_CompletedGenQueue.tryPop(generatedChunk)) {
        const int cx = generatedChunk->getChunkX();
        const int cz = generatedChunk->getChunkZ();
        const uint64_t key = getChunkKey(cx, cz);

        rendering::ChunkRendering* rawChunk = generatedChunk.get();

        {
            std::unique_lock lock(m_ChunksMutex);
            m_Chunks[key] = std::move(generatedChunk);
        }

        worldgen::WorldGen::generateTrees(*rawChunk, *this);

        m_GeneratingChunkKeys.erase(key);
        rawChunk->makeDirty();
        markNeighborsDirty(cx, cz);
    }

    rendering::ChunkRendering::ChunkMeshData meshData;
    while (m_CompletedMeshQueue.tryPop(meshData)) {
        const uint64_t key = getChunkKey(meshData.chunkX, meshData.chunkZ);

        std::shared_lock lock(m_ChunksMutex);
        if (auto it = m_Chunks.find(key); it != m_Chunks.end()) {
            it->second->uploadSectionGPU(meshData.subY, meshData.vertices);
        }

        const uint64_t subKey = getChunkKey(meshData.chunkX, meshData.subY, meshData.chunkZ);
        m_PendingMeshKeys.erase(subKey);
    }

    const int playerChunkX = toChunkCoord(static_cast<int>(std::floor(playerPos.x)));
    const int playerChunkZ = toChunkCoord(static_cast<int>(std::floor(playerPos.z)));
    const int renderDistance = config::SettingsManager::get().getRenderDistance();

    const int genDistance = renderDistance + 2;

    struct ChunkRequest { int cx, cz; float distSq; };
    std::vector<ChunkRequest> requests;

    for (int dx = -genDistance; dx <= genDistance; ++dx) {
        for (int dz = -genDistance; dz <= genDistance; ++dz) {
            const int cx = playerChunkX + dx;
            const int cz = playerChunkZ + dz;
            const uint64_t key = getChunkKey(cx, cz);

            {
                std::shared_lock lock(m_ChunksMutex);
                if (m_Chunks.contains(key) || m_GeneratingChunkKeys.contains(key)) continue;
            }

            const auto distSq = static_cast<float>(dx * dx + dz * dz);
            requests.push_back({.cx = cx, .cz = cz, .distSq = distSq});
        }
    }

    std::ranges::sort(requests, [](const ChunkRequest& a, const ChunkRequest& b) {
        return a.distSq < b.distSq;
    });

    for (const auto& req : requests) {
        const uint64_t key = getChunkKey(req.cx, req.cz);
        m_GeneratingChunkKeys.insert(key);
        m_ThreadPool.enqueue([this, cx = req.cx, cz = req.cz]() {
            auto chunk = std::make_unique<rendering::ChunkRendering>(cx, cz);
            if (!loadChunk(cx, cz, chunk.get())) {
                m_WorldGen.generateChunkData(*chunk);
            }
            m_CompletedGenQueue.push(std::move(chunk));
        });
    }

    std::shared_lock lock(m_ChunksMutex);
    for (auto& chunk : m_Chunks | std::views::values) {
        chunk->updateVisibility(deltaTime);
        if (chunk->isDirty()) {
            const int cX = chunk->getChunkX();
            const int cZ = chunk->getChunkZ();

            NeighborChunks snapshot{
                .center = chunk.get(),
                .north  = getChunkUnlocked(cX, cZ + 1),
                .south  = getChunkUnlocked(cX, cZ - 1),
                .east   = getChunkUnlocked(cX + 1, cZ),
                .west   = getChunkUnlocked(cX - 1, cZ)
            };

            if (!snapshot.north || !snapshot.south || !snapshot.east || !snapshot.west) {
                continue;
            }

            bool allQueued = true;

            for (int subY = 0; subY < 16; ++subY) {
                const uint64_t subKey = getChunkKey(cX, subY, cZ);

                if (m_PendingMeshKeys.contains(subKey)) {
                    allQueued = false;
                    continue;
                }

                m_PendingMeshKeys.insert(subKey);

                m_ThreadPool.enqueue([this, snapshot, subY]() {
                    if (snapshot.center) {
                        auto result = snapshot.center->buildSectionMeshDataCPU(snapshot, subY);
                        m_CompletedMeshQueue.push(std::move(result));
                    }
                });
            }

            if (allQueued) {
                chunk->clearDirty();
            }
        }
    }
}

void World::render(const shaders::Shader& shader, const glm::mat4& viewProjection) {
    rendering::Frustum frustum;
    frustum.update(viewProjection);

    std::shared_lock lock(m_ChunksMutex);
    for (const auto& chunk : m_Chunks | std::views::values) {
        const int cx = chunk->getChunkX();
        const int cz = chunk->getChunkZ();

        for (int subY = 0; subY < 16; ++subY) {
            const auto& sub = chunk->getSubChunks()[subY];
            if (sub.vertexCount == 0 || sub.vao == 0) continue;

            if (const rendering::BoundingBox box = chunk->getSubChunkBoundingBox(subY); !frustum.isBoxVisible(box)) continue;

            const glm::vec3 subChunkOrigin(static_cast<float>(cx) * 16.0f, static_cast<float>(subY) * 16.0f,
                static_cast<float>(cz) * 16.0f);
            shader.setVec3("u_ChunkOrigin", subChunkOrigin);
            shader.setFloat("u_ChunkVisibility", sub.visibility);

            glBindVertexArray(sub.vao);
            glDrawArrays(GL_TRIANGLES, 0, sub.vertexCount);
        }
    }
    glBindVertexArray(0);
}

}
