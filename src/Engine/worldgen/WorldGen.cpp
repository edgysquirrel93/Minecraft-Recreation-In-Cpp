#include "WorldGen.h"

#include "Engine/config/SettingsManager.h"
#include "Engine/rendering/ChunkRendering.h"


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

void WorldGen::generateChunkData(rendering::ChunkRendering& chunk) const {
    const int cx = chunk.getChunkX();
    const int cz = chunk.getChunkZ();

    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            const double wx = cx * 16 + x;
            const double wz = cz * 16 + z;

            const double baseNoise = m_Perlin.octave2D_11(wx * 0.005, wz * 0.005, 3);
            const double baseHeight = 64.0 + (baseNoise * 35.0);

            int solidBlocksAbove = 0;

            for (int y = 255; y >= 0; --y) {
                uint8_t blockID = blockregistry::ID_AIR;

                if (y == 0) {
                    blockID = blockregistry::ID_BEDROCK;
                } else {
                    constexpr int SEA_LEVEL {62};
                    const double noise3D {m_Perlin.octave3D_11(wx * 0.02, y * 0.025, wz * 0.02, 4)};

                    if (const double density = (baseHeight - y) + (noise3D * 20.0); density > 0.0) {
                        if (solidBlocksAbove == 0) {
                            if (y >= SEA_LEVEL - 1) {
                                blockID = blockregistry::ID_GRASS;
                            } else {
                                blockID = blockregistry::ID_SAND;
                            }
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
                }

                chunk.setBlock(x, y, z, blockID);
            }
        }
    }
    chunk.makeDirty();
    chunk.clearModified();
}
} // engine::worldgen