#include "WorldGen.h"

#include <fstream>
#include <ranges>
#include <print>

#include "Engine/config/SettingsManager.h"


namespace engine::worldgen {

void World::saveChunk(int cx, int cz, const ChunkRendering* chunk, const std::string& currentWorldName)
{
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
    } else if (std::filesystem::exists(tempFile)) {
        std::filesystem::remove(tempFile);
    }
}
bool World::loadChunk(int cx, int cz, ChunkRendering* chunk, const std::string& currentWorldName) {
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
        return true;
    }

    return static_cast<bool>(in);
}

void World::saveAllChunks() {
    const std::string worldName = config::LevelData::get().getCurrentWorldName();
    for (const auto& chunk : m_Chunks | std::views::values) {
        saveChunk(chunk->getChunkX(), chunk->getChunkZ(), chunk.get(), worldName);
    }
}

const block::BlockType& World::getBlockAt(const int worldX, const int worldY, const int worldZ) const {
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
                if (!loadChunk(x, z, chunk.get(), config::LevelData::get().getCurrentWorldName())) {
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
            saveChunk(it->second->getChunkX(), it->second->getChunkZ(), it->second.get(), config::LevelData::get().getCurrentWorldName());
            it = m_Chunks.erase(it);
            } else {
                ++it;
        }
    }
}

WorldGen::WorldGen(const int64_t seed)
    : m_Perlin(hashSeed64(seed)), m_Seed(seed) {}

siv::PerlinNoise::seed_type WorldGen::hashSeed64(const int64_t seed) noexcept {
    auto x = static_cast<uint64_t>(seed);
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

void WorldGen::generateChunkData(ChunkRendering& chunk) const {
    const int cx = chunk.getChunkX();
    const int cz = chunk.getChunkZ();

    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            const double wx = cx * 16 + x;
            const double wz = cz * 16 + z;

            const double noiseValue = m_Perlin.octave2D_01(wx * 0.01, wz * 0.01, 4);
            const int height = std::clamp(static_cast<int>(60.0 + noiseValue * 40.0), 0, 255);

            for (int y = 0; y <= height; ++y) {
                uint8_t blockID = blockregistry::ID_AIR;

                if (y < 5)               blockID = blockregistry::ID_BEDROCK;
                else if (y < height - 4) blockID = blockregistry::ID_STONE;
                else if (y < height)     blockID = blockregistry::ID_DIRT;
                else if (y == height)    blockID = blockregistry::ID_GRASS;

                chunk.setBlock(x, y, z, blockID);
            }
        }
    }
    chunk.makeDirty();
}
} // engine::worldgen