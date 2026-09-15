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

void ChunkRendering::updateVisibility(const float deltaTime) {
    for (auto& [vao, vbo, vertexCount, visibility] : m_SubChunks) {
        if (visibility < 1.0f) {
            visibility = std::min(1.0f, visibility + deltaTime * 5.0f);
        }
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

ChunkRendering::ChunkMeshData ChunkRendering::buildSectionMeshDataCPU(const world::NeighborChunks& neighbors, const int subY) const {
    ChunkMeshData data{ .chunkX = m_ChunkX, .chunkZ = m_ChunkZ, .subY = subY };

    const int minY = subY * 16;
    const int maxY = minY + 16;

    auto getNeighborBlock = [&](const int nx, const int ny, const int nz) -> const block::BlockType& {
        if (ny < 0 || ny >= 256) {
            return blockregistry::get(blockregistry::ID_AIR);
        }

        if (nx >= 0 && nx < 16 && nz >= 0 && nz < 16) {
            return getBlockAt(nx, ny, nz);
        }

        const ChunkRendering* target = nullptr;
        int lx = nx;
        int lz = nz;

        if (nx < 0)        { target = neighbors.west;  lx = nx + 16; }
        else if (nx >= 16) { target = neighbors.east;  lx = nx - 16; }
        else if (nz < 0)   { target = neighbors.south; lz = nz + 16; }
        else if (nz >= 16) { target = neighbors.north; lz = nz - 16; }

        if (target) {
            return target->getBlockAt(lx, ny, lz);
        }
        return blockregistry::get(blockregistry::ID_AIR);
    };

    for (int x = 0; x < 16; ++x) {
        for (int y = minY; y < maxY; ++y) {
            for (int z = 0; z < 16; ++z) {
                const block::BlockType& block = getBlockAt(x, y, z);
                if (block == blockregistry::get(blockregistry::ID_AIR)) continue;

                const int localY = y - minY;

                for (int face = 0; face < 6; ++face) {
                    const glm::ivec3 dir{NEIGHBORS[face]};
                    const int nx = x + dir.x;
                    const int ny = y + dir.y;
                    const int nz = z + dir.z;

                    if (const block::BlockType& neighborBlock = getNeighborBlock(nx, ny, nz); !neighborBlock.isOpaque && neighborBlock != block) {
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

    auto& [vao, vbo, vertexCount, visibility] = m_SubChunks[subY];

    if (vao == 0) {
        visibility = 0.0f;
    } else {
        visibility = 1.0f;
    }

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

    static constexpr int UV_INDICES[6] = { 0, 1, 2, 2, 3, 0 };

    for (int i = 0; i < 6; ++i) {
        const int cornerIdx = QUAD_INDICES[i];
        const int uvIdx = UV_INDICES[i];

        const glm::ivec3 cornerOffset = FACE_VERTS[face][cornerIdx];

        const auto vx = static_cast<uint32_t>(lx + cornerOffset.x);
        const auto vy = static_cast<uint32_t>(ly + cornerOffset.y);
        const auto vz = static_cast<uint32_t>(lz + cornerOffset.z);

        const uint32_t packed = packVertex(
            vx, vy, vz,
            static_cast<uint32_t>(face),
            static_cast<uint32_t>(uvIdx),
            texLayer
        );

        vertices.push_back(PackedVertex{ .data = packed });
    }
}

}
