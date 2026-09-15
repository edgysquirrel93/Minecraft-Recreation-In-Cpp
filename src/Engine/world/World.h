#ifndef MINECRAFT_RECREATION_RECREATION_WORLD_H
#define MINECRAFT_RECREATION_RECREATION_WORLD_H
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>

#include "Engine/block/Block.h"
#include "Engine/rendering/ChunkRendering.h"
#include "Engine/shaders/shaders.h"
#include "Engine/worldgen/WorldGen.h"
#include "Engine/util/ThreadPool.h"
#include "Engine/util/ConcurrentQueue.h"

namespace engine::rendering
{
    class ShaderManager;
    class ChunkRendering;
}

namespace engine::world {
class World {
    util::ConcurrentQueue<rendering::ChunkRendering::ChunkMeshData> m_CompletedMeshQueue;
    util::ConcurrentQueue<std::unique_ptr<rendering::ChunkRendering>> m_CompletedGenQueue;
    std::unordered_set<uint64_t> m_PendingMeshKeys;
    std::unordered_set<uint64_t> m_GeneratingChunkKeys;
    util::ThreadPool m_ThreadPool;
    mutable std::shared_mutex m_ChunksMutex;
    std::unordered_map<uint64_t, std::unique_ptr<rendering::ChunkRendering>> m_Chunks;
    worldgen::WorldGen m_WorldGen;
    static constexpr uint32_t CHUNK_FILE_MAGIC {0x564F584C};
    static constexpr uint16_t CHUNK_FILE_VERSION {1};

    struct ChunkHeader {
        uint32_t magic{CHUNK_FILE_MAGIC};
        uint16_t version{CHUNK_FILE_VERSION};
        uint32_t dataSize{0};
    };
public:

    World();
    ~World() = default;

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    static uint64_t getChunkKey(const int chunkX, const int chunkZ) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(chunkX)) << 32) |
            static_cast<uint32_t>(chunkZ);
    }

    static uint64_t getChunkKey(const int chunkX, const int chunkY, const int chunkZ) {
        const auto x = static_cast<uint64_t>(chunkX & 0x3FFFFF);
        const auto z = static_cast<uint64_t>(chunkZ & 0x3FFFFF);
        const auto y = static_cast<uint64_t>(chunkY & 0xFFFFF);
        return (x << 42) | (z << 20) | y;
    }

    static int toChunkCoord(const int worldCoord) {
        return static_cast<int>(std::floor(static_cast<float>(worldCoord) / 16.0f));
    }

    static int toLocalCoord(const int worldCoord) {
        const int local = worldCoord % 16;
        return local < 0 ? local + 16 : local;
    }

    [[nodiscard]] const rendering::ChunkRendering* getChunk(int chunkX, int chunkZ) const;
    [[nodiscard]] rendering::ChunkRendering* getChunk(int chunkX, int chunkZ);
    [[nodiscard]] const rendering::ChunkRendering* getChunk(int chunkX, int chunkY, int chunkZ) const;
    [[nodiscard]] rendering::ChunkRendering* getChunk(int chunkX, int chunkY, int chunkZ);

    void markNeighborsDirty(int chunkX, int chunkZ);

    [[nodiscard]] const block::BlockType& getBlockAt(int worldX, int worldY, int worldZ) const;
    [[nodiscard]] uint8_t getBlockIDAt(int worldX, int worldY, int worldZ) const;

    bool isChunkPendingMesh(const int chunkX, const int chunkZ) const { const uint64_t key = getChunkKey(chunkX, chunkZ);
        return m_PendingMeshKeys.contains(key); }

    void setBlockAt(int worldX, int worldY, int worldZ, uint16_t blockID);

    void update(const glm::vec3& playerPos, util::ThreadPool& threadPool);

    void render(const shaders::Shader& shader, const glm::mat4& viewProjection);

    static void saveChunk(int cx, int cz, const rendering::ChunkRendering* chunk);
    static bool loadChunk(int cx, int cz, rendering::ChunkRendering* chunk);
    void saveAllChunks();
};
}

#endif //MINECRAFT_RECREATION_RECREATION_WORLD_H
