#ifndef MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#define MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#include <PerlinNoise.hpp>

namespace engine::rendering
{
    class ChunkRendering;
}

namespace engine::worldgen
{
class WorldGen {
    siv::PerlinNoise m_Perlin;
    int64_t m_Seed;
    [[nodiscard]] static siv::PerlinNoise::seed_type hashSeed64(int64_t seed) noexcept;

public:
    explicit WorldGen(int64_t seed = 1337LL);
    void generateChunkData(rendering::ChunkRendering& chunk) const;
};
} // engine::worldgen

#endif
