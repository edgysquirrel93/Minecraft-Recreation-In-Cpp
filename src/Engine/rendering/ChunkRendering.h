#ifndef MINECRAFT_RECREATION_RECREATION_CHUNKRENDER_H
#define MINECRAFT_RECREATION_RECREATION_CHUNKRENDER_H
#include "../block/Block.h"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

#include "Frustum.h"

namespace engine::world {
    class World;
}

namespace engine::rendering {
class ChunkRendering {

    using BlockArray = std::array<uint8_t, 16 * 256 * 16>;
    int m_ChunkX{0};
    int m_ChunkZ{0};
    std::array<uint8_t, 16 * 256 * 16> m_BlockIDs{};
    bool m_IsDirty{false};
    bool m_IsModified{false};

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
    struct PackedVertex {
        uint32_t data;
    };

    static uint32_t packVertex(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t face,
                               const uint32_t uv, const uint32_t texIndex) {
        return (x & 0x1F)           |
               ((y & 0x1F) << 5)    |
               ((z & 0x1F) << 10)   |
               ((face & 0x07) << 15)|
               ((uv & 0x03) << 18)  |
               ((texIndex & 0xFFF) << 20);
    }

    struct ChunkMeshData {
        int chunkX{0};
        int chunkZ{0};
        int subY{0};
        std::vector<PackedVertex> vertices;
    };

    struct SubChunk {
        GLuint vao{0};
        GLuint vbo{0};
        GLsizei vertexCount{0};
    };

    ChunkRendering(const int chunkX, const int chunkZ) : m_ChunkX(chunkX), m_ChunkZ(chunkZ) {}
    ~ChunkRendering();

    [[nodiscard]] BoundingBox getSubChunkBoundingBox(int subY) const noexcept;

    [[nodiscard]] ChunkMeshData buildSectionMeshDataCPU(const world::World& world, int subY) const;
    void uploadSectionGPU(int subY, const std::vector<PackedVertex>& vertices);
    static void addFaceVertices(std::vector<PackedVertex>& vertices, int lx, int ly, int lz,
        int face, const block::BlockType& block);

    // getters/setters
    [[nodiscard]] const std::array<SubChunk, 16>& getSubChunks() const noexcept { return m_SubChunks; }
    [[nodiscard]] uint8_t getBlockID(const int x, const int y, const int z) const {
        if (x < 0 || x >= 16 || y < 0 || y >= 256 || z < 0 || z >= 16) return 0; return m_BlockIDs[getIndex(x, y, z)];}
    [[nodiscard]] static constexpr int getIndex(const int x, const int y, const int z) {return x + 16 * (z + 16 * y);}
    [[nodiscard]] BlockArray& getBlockIDs() { return m_BlockIDs; }
    [[nodiscard]] const BlockArray& getBlockIDs() const noexcept { return m_BlockIDs; }
    [[nodiscard]] const block::BlockType& getBlockAt(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint8_t blockID);
    [[nodiscard]] bool isDirty() const { return m_IsDirty; }
    void makeDirty() { m_IsDirty = true; }
    void clearDirty() { m_IsDirty = false; }
    [[nodiscard]] GLuint getVertex() const { return m_VertexCount; }
    [[nodiscard]] GLuint getVAO() const { return m_ChunkVAO; }
    [[nodiscard]] int getChunkX() const { return m_ChunkX; }
    [[nodiscard]] int getChunkZ() const { return m_ChunkZ; }
    [[nodiscard]] bool isModified() const noexcept { return m_IsModified; }
    void clearModified() noexcept { m_IsModified = false; }
private:
    std::array<SubChunk, 16> m_SubChunks{};
};
}

#endif
