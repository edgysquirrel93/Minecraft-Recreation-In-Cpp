#ifndef MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#define MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#include <memory>
#include <unordered_map>

#include "ChunkRendering.h"

namespace engine::worldgen
{
class World {
    static uint64_t getChunkKey(const int chunkX, const int chunkZ) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(chunkX)) << 32) |
            static_cast<uint32_t>(chunkZ);}

    std::unordered_map<uint64_t, std::unique_ptr<ChunkRendering>> m_Chunks;

public:
    ChunkRendering* getChunk(const int chunkX, const int chunkZ) {
        const uint64_t key = getChunkKey(chunkX, chunkZ);
        if (const auto it = m_Chunks.find(key); it != m_Chunks.end()) { return it->second.get(); }
        return nullptr; }
};

class WorldGen
{

};
} // engine::worldgen

#endif
