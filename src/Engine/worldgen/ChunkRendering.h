#ifndef MINECRAFT_RECREATION_RECREATION_CHUNKRENDER_H
#define MINECRAFT_RECREATION_RECREATION_CHUNKRENDER_H
#include "Engine/texture/Block.h"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

namespace engine::worldgen {
    class World;
class ChunkRendering {
    int m_ChunkX{0};
    int m_ChunkZ{0};
    std::array<uint8_t, 16 * 256 * 16> m_BlockIDs{};
    bool m_IsDirty{false};

    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoords;
        float texIndex;
    };

    const glm::ivec3 NEIGHBORS[6] = {
        { 0,  0, -1}, // Back -Z
        { 0,  0,  1}, // Front +Z
        {-1,  0,  0}, // Left -X
        { 1,  0,  0}, // Right +X
        { 0, -1,  0}, // Bottom -Y
        { 0,  1,  0}  // Top +Y
    };

    GLuint m_ChunkVAO {0}, m_ChunkVBO {0};
    GLsizei m_VertexCount{0};

public:
    ChunkRendering(const int chunkX, const int chunkZ) : m_ChunkX(chunkX), m_ChunkZ(chunkZ) {}
    ~ChunkRendering();

    void generateChunk();
    void rebuildMesh(const World& world);
    static void addFaceVertices(std::vector<Vertex>& vertices, const glm::vec3& pos, int face, const BlockType& block);

    [[nodiscard]] uint8_t getBlockID(const int x, const int y, const int z) const {
        if (x < 0 || x >= 16 || y < 0 || y >= 256 || z < 0 || z >= 16) return 0; return m_BlockIDs[getIndex(x, y, z)];}

    [[nodiscard]] static constexpr int getIndex(const int x, const int y, const int z) {return x + 16 * (z + 16 * y);}

    [[nodiscard]] const BlockType& getBlockAt(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint8_t blockID);
    [[nodiscard]] bool isDirty() const { return m_IsDirty; }
    void makeDirty() { m_IsDirty = true; }
    void clearDirty() { m_IsDirty = false; }
    [[nodiscard]] GLuint getVertex() const { return m_VertexCount; }
    [[nodiscard]] GLuint getVAO() const { return m_ChunkVAO; }
    [[nodiscard]] int getChunkX() const { return m_ChunkX; }
    [[nodiscard]] int getChunkZ() const { return m_ChunkZ; }
};
}

#endif
