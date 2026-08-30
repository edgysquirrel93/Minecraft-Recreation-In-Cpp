#include "Rendering.h"
#include "Engine/meshdata/MeshData.h"
#include "Engine/texture/Texture.h"
#include "Engine/window/WindowManager.h"
#include "Engine/config/SettingsManager.h"
#include "Engine/player/Input.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/worldgen/ChunkRendering.h"

namespace engine::rendering
{

void Rendering::drawBlock(const BlockType& blockType, const glm::vec3& position, ShaderManager& shaderManager) {
    const auto* mainShader {shaderManager.get("main")};
    if (!mainShader) return;

    const auto model {glm::translate(glm::mat4(1.0f), position)};
    mainShader->setMat4("model", model);

    if (const GLint loc {glGetUniformLocation(mainShader->ID, "u_FaceLayers")}; loc != -1) {
        glUniform1iv(loc, 6, blockType.faceLayers.data());
    }

    glBindVertexArray(meshdata::MeshData::blockVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Rendering::gameRender(ShaderManager& shaderManager, GLFWwindow* window) {
    renderMainShader(shaderManager, window);
    renderCrosshair(shaderManager, window);
    renderSelectionBox(shaderManager, window);
}

void Rendering::renderMainShader(ShaderManager& shaderManager, GLFWwindow* window) {
    int width, height;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (const auto* mainShader = shaderManager.get("main")) {
        mainShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(texture::LoadTexture::block.atlas));
        mainShader->setInt("atlas", 0);

        static bool initialized {false};
        if (!initialized) {
            m_Chunk.generateTestChunk();
            initialized = true;
        }

        if (m_Chunk.isDirty()) {
            m_Chunk.rebuildMesh();
            m_Chunk.clearDirty();
        }

        glfwGetFramebufferSize(window, &width, &height);

        const glm::mat4 projection {glm::perspective(
                glm::radians(input::Player::getTargetFov()),
                    static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1),
                    0.1f, 100.0f)};
        mainShader->setMat4("projection", projection);

        constexpr auto upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 cameraPos   = config::LevelData::get().getCameraPos();
        const glm::vec3 cameraView = input::Camera::getCameraFront();

        m_View = glm::lookAt(cameraPos, cameraPos + cameraView, upVector);
        mainShader->setMat4("view", m_View);

        mainShader->setMat4("model", glm::mat4(1.0f));

        if (m_Chunk.getVertex() > 0) {
            glBindVertexArray(m_Chunk.getVAO());
            glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_Chunk.getVertex()));
        }
    }
}

void Rendering::renderCrosshair(ShaderManager& shaderManager, GLFWwindow* window) {

    int width, height;

    if (const auto* crosshairShader = shaderManager.get("crosshair")) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        float aspect = 1.0f;
        aspect = static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1);
        crosshairShader->use();
        crosshairShader->setFloat("aspectRatio", aspect);
        glBindVertexArray(meshdata::MeshData::crossVAO);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glDisable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glEnable(GL_DEPTH_TEST);
    }
}

void Rendering::renderSelectionBox(ShaderManager& shaderManager, GLFWwindow* window) {
    if (const auto* selectionBoxShader = shaderManager.get("selectionBox")) {
        selectionBoxShader->use();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const glm::mat4 projection = glm::perspective(
            glm::radians(input::Player::getTargetFov()),
            static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1),
            0.1f, 100.0f
        );

        selectionBoxShader->setMat4("projection", projection);
        selectionBoxShader->setMat4("view", m_View);

        if (const auto raycast = input::Camera::raycast(config::LevelData::get().getCameraPos(),
            input::Camera::getCameraFront(), 5.0f, m_Chunk); raycast.hit) {

            glm::vec3 blockCenter = glm::vec3(raycast.blockPos) + glm::vec3(0.5f);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), blockCenter);
            model = glm::scale(model, glm::vec3(1.002f));

            selectionBoxShader->setMat4("model", model);
            selectionBoxShader->setVec3("color", glm::vec3(0.0f, 0.0f, 0.0f));

            glBindVertexArray(meshdata::MeshData::selVAO);
            glLineWidth(2.5f);
            glDrawArrays(GL_LINES, 0, 24);
            }
    }
}
} // engine::rendering