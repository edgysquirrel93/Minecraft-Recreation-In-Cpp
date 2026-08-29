#ifndef MINECRAFT_RECREATION_RECREATION_CHUNKRENDER_H
#define MINECRAFT_RECREATION_RECREATION_CHUNKRENDER_H
#include "Engine/texture/Block.h"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

namespace engine::worldgen
{
class ChunkRendering {
    BlockType m_TestBlocks[16][256][16] {{{}}};
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

    GLuint m_ChunkVAO = {}, m_ChunkVBO = {};
    GLsizei m_VertexCount{0};

public:
    void generateTestChunk();
    void rebuildMesh();
    static void addFaceVertices(std::vector<Vertex>& vertices, const glm::vec3& pos, int face, const BlockType& block);

    BlockType getBlockAt(int x, int y, int z);
    void setBlock(int x, int y, int z, const BlockType& block);
    [[nodiscard]] bool isDirty() const { return m_IsDirty; }
    void clearDirty() { m_IsDirty = false; }
    [[nodiscard]] GLuint getVertex() const { return m_VertexCount; }

    [[nodiscard]] const auto& getTestBlocks() const { return m_TestBlocks; }
};
}

#endif
