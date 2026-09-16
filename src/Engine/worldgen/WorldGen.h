#ifndef MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#define MINECRAFT_RECREATION_RECREATION_WORLDGEN_H
#include <PerlinNoise.hpp>

namespace engine::rendering
{
    class ChunkRendering;
}

namespace engine::world {
    class World;
}

namespace engine::worldgen
{
class WorldGen {
    siv::PerlinNoise m_Perlin;
    int64_t m_Seed;
    [[nodiscard]] static siv::PerlinNoise::seed_type hashSeed64(int64_t seed) noexcept;

    // tree generating
    static bool treeHasHeadroom(const world::World& world, int x, int y, int z);
    static void placeTreeStructure(world::World& world, int x, int y, int z);

public:
    explicit WorldGen(int64_t seed = 1337LL);

    static int findGroundLevel(const rendering::ChunkRendering& chunk, int x, int z);
    static void generateTrees(const rendering::ChunkRendering& chunk, const world::World& world);

    void generateChunkData(rendering::ChunkRendering& chunk) const;
};
} // engine::worldgen

#endif
