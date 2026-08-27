#include "MeshData.h"
#include <glad/gl.h>


namespace engine::meshdata
{
    // Very large annoying arrays
    inline constexpr float block[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    inline constexpr float crosshair[] = {
        // Vertical Bar
        -0.0025f,  0.025f, 0.0f,   0.0025f,  0.025f, 0.0f,  -0.0025f, -0.025f, 0.0f,
        -0.0025f, -0.025f, 0.0f,   0.0025f,  0.025f, 0.0f,   0.0025f, -0.025f, 0.0f,

        // Horizontal Bar
        -0.025f,   0.0025f, 0.0f,   0.025f,   0.0025f, 0.0f,  -0.025f,  -0.0025f, 0.0f,
        -0.025f,  -0.0025f, 0.0f,   0.025f,   0.0025f, 0.0f,   0.025f,  -0.0025f, 0.0f
    };

    inline constexpr float selectionCube[] = {
        // Bottom square
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,
        // Top square
        0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        // Vertical pillars
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f
    };
    // Initialisation
    void MeshData::Init()
    {
        // Selection Cube
        glGenVertexArrays(1, &selVAO);
        glGenBuffers(1, &selVBO);
        glBindVertexArray(selVAO);
        glBindBuffer(GL_ARRAY_BUFFER, selVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(selectionCube), selectionCube, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
        glEnableVertexAttribArray(0);

        // 2. Crosshair
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_ONE, GL_ONE);
        glGenVertexArrays(1, &crossVAO);
        glGenBuffers(1, &crossVBO);
        glBindVertexArray(crossVAO);
        glBindBuffer(GL_ARRAY_BUFFER, crossVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair), crosshair, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
        glEnableVertexAttribArray(0);
        glDisable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);

        // Block
        glGenVertexArrays(1, &blockVAO);
        glGenBuffers(1, &blockVBO);
        glBindVertexArray(blockVAO);
        glBindBuffer(GL_ARRAY_BUFFER, blockVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(block), block, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(nullptr));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

UVCoord MeshData::getTileUV(const int col, const int row) {
    constexpr float tileSize = 0.25f;
    return {
        .u1 = static_cast<float>(col) * tileSize,         // u1
        .v1 = static_cast<float>(row) * tileSize,         // v1
        .u2 = static_cast<float>(col + 1) * tileSize,     // u2
        .v2 = static_cast<float>(row + 1) * tileSize      // v2
    };
}
} // engine::meshdata