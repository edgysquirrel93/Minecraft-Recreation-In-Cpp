#include "WorldGen.h"

#include "Engine/config/SettingsManager.h"
#include "Engine/rendering/ChunkRendering.h"
#include "Engine/world/World.h"

namespace engine::worldgen {

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

double WorldGen::getPlainsHeight(const double wx, const double wz) const {
    return m_Perlin.octave2D_11(wx * 0.006, wz * 0.006, 3) * 6.0;
}

double WorldGen::getContinentalness(const double wx, const double wz) const {
    const double rawContinent = m_Perlin.octave2D_11(wx * 0.002, wz * 0.002, 3);
    return std::pow(std::abs(rawContinent), 1.2) * (rawContinent < 0 ? -1.0 : 1.0);
}

    double WorldGen::getMountainHeight(const double wx, const double wz) const {
    const double broadBase = std::abs(m_Perlin.octave2D_11(wx * 0.003, wz * 0.003, 2));

    const double ridge1 = 1.0 - std::abs(m_Perlin.octave2D_11(wx * 0.008, wz * 0.008, 3));
    const double ridge2 = 1.0 - std::abs(m_Perlin.octave2D_11(wx * 0.02, wz * 0.02, 2));

    const double peakShape = (broadBase * 0.35) + (std::pow(ridge1, 1.4) * (ridge2 * 0.5 + 0.5) * 0.65);

    return peakShape * 135.0;
}
double WorldGen::getTerrainHeight(const double wx, const double wz) const {
    constexpr double SEA_LEVEL = 62.0;
    const double continent = getContinentalness(wx, wz);

    const double mountainWeight = std::clamp((continent + 0.05) * 1.5, 0.0, 1.0);

    const double plains = getPlainsHeight(wx, wz);
    const double mountains = getMountainHeight(wx, wz) * mountainWeight;

    return SEA_LEVEL + plains + mountains;
}

bool WorldGen::isCliff(const double wx, const double wz, const double currentHeight) const {
    const double neighborHeight = getTerrainHeight(wx + 2.0, wz);
    return std::abs(currentHeight - neighborHeight) > 5.0;
}

bool WorldGen::isCaveAt(const double wx, const int y, const double wz) const {
    const double cave1 = m_Perlin.octave3D_11(wx * 0.025, y * 0.025, wz * 0.025, 2);
    const double cave2 = m_Perlin.octave3D_11((wx + 75.0) * 0.025, y * 0.025, (wz + 75.0) * 0.025, 2);
    return (std::abs(cave1) < 0.12 && std::abs(cave2) < 0.12);
}

uint8_t WorldGen::getBlockType(const int y, const int surfaceHeight, const bool isCave) {
    constexpr int SEA_LEVEL{62};

    if (isCave) {
        return blockregistry::ID_AIR;
    }

    if (y == surfaceHeight) {
        return (surfaceHeight <= SEA_LEVEL) ? blockregistry::ID_SAND : blockregistry::ID_GRASS;
    }
    if (y >= surfaceHeight - 3) {
        return (surfaceHeight <= SEA_LEVEL) ? blockregistry::ID_SAND : blockregistry::ID_DIRT;
    }
    return blockregistry::ID_STONE;
}

int WorldGen::findGroundLevel(const rendering::ChunkRendering& chunk, const int x, const int z) {
    for (int y = 255; y >= 0; --y) {
        if (const auto& block = chunk.getBlockAt(x, y, z); block != blockregistry::get(blockregistry::ID_AIR)) {
            return y;
        }
    }
    return -1;
}

bool WorldGen::treeHasHeadroom(const world::World& world, const int x, const int y, const int z) {

    for (int cy = y; cy <= y + 4; ++cy) {
        if (world.getBlockIDAt(x, cy, z) != blockregistry::ID_AIR) {
            return false;
        }
    }

    for (int cy = y + 2; cy <= y + 3; ++cy) {
        for (int cx = x - 2; cx <= x + 2; ++cx) {
            for (int cz = z - 2; cz <= z + 2; ++cz) {
                if (const uint8_t blockID = world.getBlockIDAt(cx, cy, cz); blockID != blockregistry::ID_AIR) {
                    return false;
                }
            }
        }
    }

    for (int cy = y + 4; cy <= y + 5; ++cy) {
        for (int cx = x - 1; cx <= x + 1; ++cx) {
            for (int cz = z - 1; cz <= z + 1; ++cz) {
                if (const uint8_t blockID = world.getBlockIDAt(cx, cy, cz); blockID != blockregistry::ID_AIR) {
                    return false;
                }
            }
        }
    }

    return true;
}

void WorldGen::placeTreeStructure(world::World& world, const int x, const int y, const int z) {
    for (int cx = x - 2; cx <= x + 2; cx++) {
        for (int cz = z - 2; cz <= z + 2; cz++) {
            world.setBlockWorldGen(cx, y + 2, cz, blockregistry::ID_OAK_LEAVES);
        }
    }

    for (int cx = x - 2; cx <= x + 2; cx++) {
        for (int cz = z - 2; cz <= z + 2; cz++) {
            world.setBlockWorldGen(cx, y + 3, cz, blockregistry::ID_OAK_LEAVES);
        }
    }

    for (int cx = x - 1; cx <= x + 1; cx++) {
        for (int cz = z - 1; cz <= z + 1; cz++) {
            world.setBlockWorldGen(cx, y + 4, cz, blockregistry::ID_OAK_LEAVES);
        }
    }

    for (int cx = x - 1; cx <= x + 1; cx++) {
        for (int cz = z - 1; cz <= z + 1; cz++) {
            if (std::abs(cx - x) == 1 && std::abs(cz - z) == 1) continue;

            world.setBlockWorldGen(cx, y + 5, cz, blockregistry::ID_OAK_LEAVES);
        }
    }

    for (int cy = y; cy < y + 5; cy++) {
        world.setBlockWorldGen(x, cy, z, blockregistry::ID_OAK_LOG);
    }
}

void WorldGen::generateTrees(const rendering::ChunkRendering& chunk, const world::World& world) const {
    const int chunkX = chunk.getChunkX();
    const int chunkZ = chunk.getChunkZ();

    std::mt19937 rng(static_cast<unsigned int>(chunkX * 73856093 ^ chunkZ * 19349663));
    std::uniform_real_distribution dist(0.0f, 1.0f);

    for (int x = 2; x < 14; ++x) {
        for (int z = 2; z < 14; ++z) {
            const int worldX = (chunkX * 16) + x;
            const int worldZ = (chunkZ * 16) + z;

            const double forestMask = m_Perlin.octave2D_11(worldX * 0.003, worldZ * 0.003, 2);

            float spawnChance = 0.0005f;
            if (forestMask > 0.35) {
                spawnChance = 0.025f;
            }

            if (dist(rng) > spawnChance) continue;

            const int surfaceY = findGroundLevel(chunk, x, z);
            if (surfaceY == -1) continue;

            if (chunk.getBlockAt(x, surfaceY, z) != blockregistry::get(blockregistry::ID_GRASS)) continue;
            if (!treeHasHeadroom(world, worldX, surfaceY + 1, worldZ)) continue;

            placeTreeStructure(const_cast<world::World&>(world), worldX, surfaceY + 1, worldZ);
        }
    }
}

void WorldGen::generateChunkData(rendering::ChunkRendering& chunk) const {
    const int cx = chunk.getChunkX();
    const int cz = chunk.getChunkZ();

    for (int x = 0; x < 16; ++x) {
        const double wx = cx * 16 + x;
        for (int z = 0; z < 16; ++z) {
            const double wz = cz * 16 + z;

            const double surfaceHeightDbl = getTerrainHeight(wx, wz);
            const int surfaceHeight = static_cast<int>(std::round(surfaceHeightDbl));

            const double surfaceOpenNoise = m_Perlin.octave2D_11(wx * 0.03, wz * 0.03, 2);
            const bool allowSurfaceOpening = (surfaceOpenNoise > 0.65);

            const int caveBuffer = allowSurfaceOpening ? 0 : 3;

            for (int y = 255; y >= 0; --y) {
                uint8_t blockID = blockregistry::ID_AIR;

                if (y <= 4) {
                    blockID = blockregistry::ID_BEDROCK;
                }
                else if (y > surfaceHeight)
                {
                    constexpr int SEA_LEVEL{62};
                    blockID = (y <= SEA_LEVEL) ? blockregistry::ID_WATER : blockregistry::ID_AIR;
                }
                else {
                    const bool isCave = isCaveAt(wx, y, wz) && (y < surfaceHeight - caveBuffer) && (y > 8);

                    blockID = getBlockType(y, surfaceHeight, isCave);
                }

                chunk.setBlock(x, y, z, blockID);
            }
        }
    }

    chunk.makeDirty();
    chunk.clearModified();
}

} // engine::worldgen