#include "ChunkRendering.h"

#include "Engine/rendering/Rendering.h"
#include "Engine/world/World.h"

namespace engine::rendering {

ChunkRendering::~ChunkRendering() {
    for (auto& sub : m_SubChunks) {
        if (sub.vao != 0) glDeleteVertexArrays(1, &sub.vao);
        if (sub.vbo != 0) glDeleteBuffers(1, &sub.vbo);
    }
}

const block::BlockType& ChunkRendering::getBlockAt(const int x, const int y, const int z) const {
    if (x < 0 || x >= 16 || y < 0 || y >= 256 || z < 0 || z >= 16) {
        return blockregistry::get(blockregistry::ID_AIR);
    }
    const uint8_t id = m_BlockIDs[getIndex(x, y, z)];
    return blockregistry::get(id);
}

void ChunkRendering::setBlock(const int x, const int y, const int z, const uint8_t blockID) {
    if (x < 0 || x >= 16 || y < 0 || y >= 256 || z < 0 || z >= 16) return;

    if (const int index = getIndex(x, y, z); m_BlockIDs[index] != blockID) {
        m_BlockIDs[index] = blockID;
        m_IsDirty = true;
        m_IsModified = true;
    }
}

BoundingBox ChunkRendering::getSubChunkBoundingBox(const int subY) const noexcept {
    const auto minX = static_cast<float>(m_ChunkX * 16);
    const auto minY = static_cast<float>(subY * 16);
    const auto minZ = static_cast<float>(m_ChunkZ * 16);

    return BoundingBox{
        .min = {minX, minY, minZ},
        .max = {minX + 16.0f, minY + 16.0f, minZ + 16.0f}
    };
}

ChunkRendering::ChunkMeshData ChunkRendering::buildSectionMeshDataCPU(const world::World& world, const int subY) const {
    ChunkMeshData data{ .chunkX = m_ChunkX, .chunkZ = m_ChunkZ, .subY = subY };

    const int worldXOffset = m_ChunkX * 16;
    const int worldZOffset = m_ChunkZ * 16;

    const int minY = subY * 16;
    const int maxY = minY + 16;

    for (int x = 0; x < 16; x++) {
        for (int y = minY; y < maxY; y++) {
            for (int z = 0; z < 16; z++) {
                const block::BlockType& block = getBlockAt(x, y, z);
                if (block == blockregistry::get(blockregistry::ID_AIR)) continue;

                const int localY = y - minY;

                for (int face = 0; face < 6; ++face) {
                    const glm::ivec3 dir {NEIGHBORS[face]};
                    const int nx = x + dir.x;
                    const int ny = y + dir.y;
                    const int nz = z + dir.z;

                    block::BlockType neighborBlock;
                    if (nx >= 0 && nx < 16 && ny >= 0 && ny < 256 && nz >= 0 && nz < 16) {
                        neighborBlock = getBlockAt(nx, ny, nz);
                    } else {
                        neighborBlock = world.getBlockAt(worldXOffset + nx, ny, worldZOffset + nz);
                    }

                    if (!neighborBlock.isOpaque && neighborBlock != block) {
                        addFaceVertices(data.vertices, x, localY, z, face, block);
                    }
                }
            }
        }
    }
    return data;
}

void ChunkRendering::uploadSectionGPU(const int subY, const std::vector<PackedVertex>& vertices) {
    if (subY < 0 || subY >= 16) return;

    auto& [vao, vbo, vertexCount] = m_SubChunks[subY];

    if (vertices.empty()) {
        vertexCount = 0;
        if (vbo != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        }
        return;
    }

    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(PackedVertex), static_cast<void*>(nullptr));
        glEnableVertexAttribArray(0);
    } else {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(PackedVertex)), vertices.data(), GL_DYNAMIC_DRAW);
    vertexCount = static_cast<GLsizei>(vertices.size());
    m_IsDirty = false;
}
void ChunkRendering::addFaceVertices(std::vector<PackedVertex>& vertices, const int lx, const int ly, const int lz,
        const int face, const block::BlockType& block) {

    const auto texLayer = static_cast<uint32_t>(block.faceLayers[face]);

    static constexpr glm::ivec3 FACE_VERTS[6][4] = {
        // Back -Z
        { {0,0,0}, {0,1,0}, {1,1,0}, {1,0,0} },
        // Front +Z
        { {1,0,1}, {1,1,1}, {0,1,1}, {0,0,1} },
        // Left -X
        { {0,0,1}, {0,1,1}, {0,1,0}, {0,0,0} },
        // Right +X
        { {1,0,0}, {1,1,0}, {1,1,1}, {1,0,1} },
        // Bottom -Y
        { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} },
        // Top +Y
        { {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} }
    };

    static constexpr int QUAD_INDICES[6] = { 0, 1, 2, 2, 3, 0 };

    for (const int cornerIdx : QUAD_INDICES) {
        const glm::ivec3 cornerOffset = FACE_VERTS[face][cornerIdx];

        const auto vx = static_cast<uint32_t>(lx + cornerOffset.x);
        const auto vy = static_cast<uint32_t>(ly + cornerOffset.y);
        const auto vz = static_cast<uint32_t>(lz + cornerOffset.z);

        const uint32_t packed = packVertex(
            vx, vy, vz,
            static_cast<uint32_t>(face),
            static_cast<uint32_t>(cornerIdx),
            texLayer
        );

        vertices.push_back(PackedVertex{ .data = packed });
    }
}

}
