#ifndef MINECRAFT_RECREATION_RECREATION_WORLD_H
#define MINECRAFT_RECREATION_RECREATION_WORLD_H
#include <memory>
#include <unordered_map>

#include "Engine/block/Block.h"
#include "Engine/worldgen/WorldGen.h"
#include "glm/vec3.hpp"

namespace engine::rendering
{
    class ChunkRendering;
}

namespace engine::world {
class World {
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

    World(World&&) noexcept = default;
    World& operator=(World&&) noexcept = default;

    static uint64_t getChunkKey(const int chunkX, const int chunkZ) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(chunkX)) << 32) |
            static_cast<uint32_t>(chunkZ);
    }

    static int toChunkCoord(const int worldCoord) {
        return static_cast<int>(std::floor(static_cast<float>(worldCoord) / 16.0f));
    }

    static int toLocalCoord(const int worldCoord) {
        const int local = worldCoord % 16;
        return local < 0 ? local + 16 : local;
    }

    [[nodiscard]] const rendering::ChunkRendering* getChunk(const int chunkX, const int chunkZ) const {
        const uint64_t key = getChunkKey(chunkX, chunkZ);
        if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    [[nodiscard]] rendering::ChunkRendering* getChunk(const int chunkX, const int chunkZ) {
        const uint64_t key = getChunkKey(chunkX, chunkZ);
        if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    [[nodiscard]] const block::BlockType& getBlockAt(int worldX, int worldY, int worldZ) const;
    [[nodiscard]] uint8_t getBlockIDAt(int worldX, int worldY, int worldZ) const;

    void setBlockAt(int worldX, int worldY, int worldZ, uint16_t blockID);

    void update(const glm::vec3& playerPos);

    void render() const;

    static void saveChunk(int cx, int cz, const rendering::ChunkRendering* chunk);
    static bool loadChunk(int cx, int cz, rendering::ChunkRendering* chunk);
    void saveAllChunks();
};
}

#endif //MINECRAFT_RECREATION_RECREATION_WORLD_H
