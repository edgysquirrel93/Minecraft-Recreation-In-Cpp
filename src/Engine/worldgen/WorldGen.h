#ifndef MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#define MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#include <memory>
#include <ranges>
#include <unordered_map>

#include "ChunkRendering.h"

namespace engine::worldgen
{
class World {
    std::unordered_map<uint64_t, std::unique_ptr<ChunkRendering>> m_Chunks;

public:

    World() = default;
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

    [[nodiscard]] const ChunkRendering* getChunk(const int chunkX, const int chunkZ) const {
        const uint64_t key = getChunkKey(chunkX, chunkZ);
        if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    [[nodiscard]] ChunkRendering* getChunk(const int chunkX, const int chunkZ) {
        const uint64_t key = getChunkKey(chunkX, chunkZ);
        if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    [[nodiscard]] const BlockType& getBlockAt(int worldX, int worldY, int worldZ) const;

    void setBlockAt(int worldX, int worldY, int worldZ, uint16_t blockID);

    void update(const glm::vec3& playerPos);

    void render() const {
        for (const auto& chunk : m_Chunks | std::views::values) {
            if (chunk->getVertex() > 0) {
                glBindVertexArray(chunk->getVAO());
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(chunk->getVertex()));
            }}}
};

class WorldGen
{

};
} // engine::worldgen

#endif
