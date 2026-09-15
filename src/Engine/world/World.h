#ifndef MINECRAFT_RECREATION_RECREATION_WORLD_H
#define MINECRAFT_RECREATION_RECREATION_WORLD_H
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include <chrono>

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

struct NeighborChunks {
    const rendering::ChunkRendering* center{nullptr};
    const rendering::ChunkRendering* north{nullptr}; // +Z
    const rendering::ChunkRendering* south{nullptr}; // -Z
    const rendering::ChunkRendering* east{nullptr};  // +X
    const rendering::ChunkRendering* west{nullptr};  // -X
};

class World {
    util::ConcurrentQueue<rendering::ChunkRendering::ChunkMeshData> m_CompletedMeshQueue;
    util::ConcurrentQueue<std::unique_ptr<rendering::ChunkRendering>> m_CompletedGenQueue;
    std::unordered_set<uint64_t> m_PendingMeshKeys;
    std::unordered_set<uint64_t> m_GeneratingChunkKeys;
    util::ThreadPool m_ThreadPool;
    mutable std::shared_mutex m_ChunksMutex;
    std::unordered_map<uint64_t, std::unique_ptr<rendering::ChunkRendering>> m_Chunks;
    worldgen::WorldGen m_WorldGen;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime{ std::chrono::high_resolution_clock::now() };
    static constexpr uint32_t CHUNK_FILE_MAGIC {0x564F584C};
    static constexpr uint16_t CHUNK_FILE_VERSION {1};

    struct ChunkHeader {
        uint32_t magic{CHUNK_FILE_MAGIC};
        uint16_t version{CHUNK_FILE_VERSION};
        uint32_t dataSize{0};
    };

    [[nodiscard]] const rendering::ChunkRendering* getChunkUnlocked(int chunkX, int chunkZ) const;
    [[nodiscard]] rendering::ChunkRendering* getChunkUnlocked(int chunkX, int chunkZ);
    [[nodiscard]] const rendering::ChunkRendering* getChunkUnlocked(int chunkX, int chunkY, int chunkZ) const;
    [[nodiscard]] rendering::ChunkRendering* getChunkUnlocked(int chunkX, int chunkY, int chunkZ);

public:

    World();
    ~World() = default;

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    static uint64_t getChunkKey(const int x, const int z) noexcept {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) |
                static_cast<uint64_t>(static_cast<uint32_t>(z));
    }

    static uint64_t getChunkKey(const int x, const int y, const int z) noexcept {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 40) |
               (static_cast<uint64_t>(static_cast<uint32_t>(y) & 0xFF) << 32) |
                static_cast<uint64_t>(static_cast<uint32_t>(z));
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

    NeighborChunks getNeighborSnapshot(int cx, int cz) const;

    void update(const glm::vec3& playerPos);
    void render(const shaders::Shader& shader, const glm::mat4& viewProjection);

    static void saveChunk(int cx, int cz, const rendering::ChunkRendering* chunk);
    static bool loadChunk(int cx, int cz, rendering::ChunkRendering* chunk);
    void saveAllChunks();
};
}

#endif //MINECRAFT_RECREATION_RECREATION_WORLD_H
