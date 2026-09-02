#include "ChunkRendering.h"

#include "WorldGen.h"
#include "Engine/rendering/Rendering.h"

namespace engine::worldgen {

ChunkRendering::~ChunkRendering() {
    if (m_ChunkVAO != 0) glDeleteVertexArrays(1, &m_ChunkVAO);
    if (m_ChunkVBO != 0) glDeleteBuffers(1, &m_ChunkVBO);
}

void ChunkRendering::generateChunk() {
    m_BlockIDs.fill(blockregistry::ID_AIR);

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            setBlock(x, 64, z, blockregistry::ID_GRASS);

            for (int y = 59; y <= 63; y++) {
                setBlock(x, y, z, blockregistry::ID_DIRT);
            }
            for (int y = 6; y <= 58; y++) {
                setBlock(x, y, z, blockregistry::ID_STONE);
            }
            for (int y = 0; y <= 5; y++) {
                setBlock(x, y, z, blockregistry::ID_BEDROCK);
            }
        }
    }
}

const BlockType& ChunkRendering::getBlockAt(const int x, const int y, const int z) const {
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
    }
}

void ChunkRendering::rebuildMesh(const World& world) {
    std::vector<Vertex> vertices;

    const int worldXOffset = m_ChunkX * 16;
    const int worldZOffset = m_ChunkZ * 16;

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                const BlockType& block = getBlockAt(x, y, z);
                if (block == blockregistry::get(blockregistry::ID_AIR)) continue;

                const glm::vec3 worldBlockPos(x + worldXOffset, y, z + worldZOffset);

                for (int face = 0; face < 6; ++face) {
                    const glm::ivec3 dir {NEIGHBORS[face]};
                    const int nx = x + dir.x;
                    const int ny = y + dir.y;
                    const int nz = z + dir.z;

                    BlockType neighborBlock;

                    if (nx >= 0 && nx < 16 && ny >= 0 && ny < 256 && nz >= 0 && nz < 16) {
                        neighborBlock = getBlockAt(nx, ny, nz);
                    } else {
                        neighborBlock = world.getBlockAt(worldXOffset + nx, ny, worldZOffset + nz);
                    }

                    if (!neighborBlock.isOpaque && neighborBlock != block) {
                        addFaceVertices(vertices, worldBlockPos, face, block);
                    }
                }
            }
        }
    }

    if (m_ChunkVAO == 0) {
        glGenVertexArrays(1, &m_ChunkVAO);
        glGenBuffers(1, &m_ChunkVBO);
    }

    glBindVertexArray(m_ChunkVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_ChunkVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoords)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texIndex)));
    glEnableVertexAttribArray(2);

    m_VertexCount = static_cast<GLsizei>(vertices.size());
    m_IsDirty = false;
}

void ChunkRendering::addFaceVertices(std::vector<Vertex>& vertices, const glm::vec3& pos, const int face, const BlockType& block) {

    const auto texLayer = static_cast<float>(block.faceLayers[face]);

    static const glm::vec3 FACE_VERTS[6][4] = {
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

    static constexpr glm::vec2 UVs[4] = {
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}
    };

    vertices.push_back({ .position = pos + FACE_VERTS[face][0], .texCoords = UVs[0], .texIndex = texLayer });
    vertices.push_back({ .position = pos + FACE_VERTS[face][1], .texCoords = UVs[1], .texIndex = texLayer });
    vertices.push_back({ .position = pos + FACE_VERTS[face][2], .texCoords = UVs[2], .texIndex = texLayer });

    vertices.push_back({ .position = pos + FACE_VERTS[face][2], .texCoords = UVs[2], .texIndex = texLayer });
    vertices.push_back({ .position = pos + FACE_VERTS[face][3], .texCoords = UVs[3], .texIndex = texLayer });
    vertices.push_back({ .position = pos + FACE_VERTS[face][0], .texCoords = UVs[0], .texIndex = texLayer });
}

}
