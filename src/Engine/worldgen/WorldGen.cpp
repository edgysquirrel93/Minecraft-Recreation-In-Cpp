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
                if (uint8_t blockID = world.getBlockIDAt(cx, cy, cz); blockID != blockregistry::ID_AIR) {
                    return false;
                }
            }
        }
    }

    for (int cy = y + 4; cy <= y + 5; ++cy) {
        for (int cx = x - 1; cx <= x + 1; ++cx) {
            for (int cz = z - 1; cz <= z + 1; ++cz) {
                if (uint8_t blockID = world.getBlockIDAt(cx, cy, cz); blockID != blockregistry::ID_AIR) {
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

void WorldGen::generateTrees(const rendering::ChunkRendering& chunk, const world::World& world) {
    const int chunkX = chunk.getChunkX();
    const int chunkZ = chunk.getChunkZ();

    std::mt19937 rng(static_cast<unsigned int>(chunkX * 73856093 ^ chunkZ * 19349663));
    std::uniform_real_distribution dist(0.0f, 1.0f);

    for (int x = 2; x < 14; ++x) {
        for (int z = 2; z < 14; ++z) {

            if (dist(rng) > 0.003f) continue;

            const int surfaceY = findGroundLevel(chunk, x, z);
            if (surfaceY == -1) continue;

            if (const auto& surfaceBlock = chunk.getBlockAt(x, surfaceY, z); surfaceBlock != blockregistry::get(blockregistry::ID_GRASS)) continue;

            const int worldX = (chunkX * 16) + x;
            const int worldZ = (chunkZ * 16) + z;

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
        const double wx_005 = wx * 0.005;
        const double wx_02 = wx * 0.02;

        for (int z = 0; z < 16; ++z) {
            constexpr int SEA_LEVEL{62};
            const double wz = cz * 16 + z;
            const double wz_005 = wz * 0.005;
            const double wz_02 = wz * 0.02;

            const double noiseVal = m_Perlin.octave2D_11(wx_005, wz_005, 4);

            double heightFactor = 0.0;
            if (noiseVal < 0.0) {
                heightFactor = noiseVal * 0.3;
            } else {
                heightFactor = std::pow(noiseVal, 1.8) * 2.8;
            }

            const double baseHeight = 65.0 + (heightFactor * 55.0);

            const int maxY = std::min(255, static_cast<int>(std::ceil(baseHeight + 25.0)));
            const int minY = std::max(1, static_cast<int>(std::floor(baseHeight - 30.0)));

            int solidBlocksAbove = 0;

            for (int y = 255; y > maxY; --y) {
                const uint8_t blockID = (y <= SEA_LEVEL) ? blockregistry::ID_WATER : blockregistry::ID_AIR;
                chunk.setBlock(x, y, z, blockID);
            }

            for (int y = maxY; y >= minY; --y) {
                uint8_t blockID = blockregistry::ID_AIR;
                const double noise3D = m_Perlin.octave3D_11(wx_02, y * 0.025, wz_02, 4);

                if (const double density = (baseHeight - y) + (noise3D * 20.0); density > 0.0) {
                    if (solidBlocksAbove == 0) {
                        blockID = (y >= SEA_LEVEL - 1) ? blockregistry::ID_GRASS : blockregistry::ID_SAND;
                    } else if (solidBlocksAbove < 4) {
                        blockID = (y < SEA_LEVEL) ? blockregistry::ID_SAND : blockregistry::ID_DIRT;
                    } else {
                        blockID = blockregistry::ID_STONE;
                    }
                    solidBlocksAbove++;
                } else {
                    solidBlocksAbove = 0;
                    if (y <= SEA_LEVEL) {
                        blockID = blockregistry::ID_WATER;
                    }
                }

                chunk.setBlock(x, y, z, blockID);
            }

            for (int y = minY - 1; y >= 1; --y) {
                chunk.setBlock(x, y, z, blockregistry::ID_STONE);
            }

            chunk.setBlock(x, 0, z, blockregistry::ID_BEDROCK);
        }
    }

    chunk.makeDirty();
    chunk.clearModified();
}

} // engine::worldgen