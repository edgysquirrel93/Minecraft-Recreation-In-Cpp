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

    // biome and height functions
    [[nodiscard]] double getContinentalness(double wx, double wz) const;
    [[nodiscard]] double getPlainsHeight(double wx, double wz) const;
    [[nodiscard]] double getMountainHeight(double wx, double wz) const;
    [[nodiscard]] double getTerrainHeight(double wx, double wz) const;

    // features and material functions
    [[nodiscard]] bool isCliff(double wx, double wz, double currentHeight) const;
    [[nodiscard]] bool isCaveAt(double wx, int y, double wz) const;
    static uint8_t getBlockType(int y, int surfaceHeight, bool isCave) ;

    // tree generating functions, private helpers
    static bool treeHasHeadroom(const world::World& world, int x, int y, int z);
    static void placeTreeStructure(world::World& world, int x, int y, int z);

public:
    explicit WorldGen(int64_t seed = 1337LL);

    static int findGroundLevel(const rendering::ChunkRendering& chunk, int x, int z);
    void generateTrees(const rendering::ChunkRendering& chunk, const world::World& world) const;

    void generateChunkData(rendering::ChunkRendering& chunk) const;
};
} // engine::worldgen

#endif
