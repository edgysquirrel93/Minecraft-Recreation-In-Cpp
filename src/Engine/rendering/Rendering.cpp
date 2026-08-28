#include "Rendering.h"
#include "Engine/meshdata/MeshData.h"
#include "Engine/texture/Texture.h"
#include "Engine/window/WindowManager.h"
#include "Engine/config/SettingsManager.h"
#include "Engine/player/Input.h"
#include <glm/gtc/matrix_transform.hpp>

namespace engine::rendering
{

void Rendering::drawBlock(const BlockType& blockType, const glm::vec3& position, ShaderManager& shaderManager) {
    const auto* mainShader {shaderManager.get("main")};
    if (!mainShader) return;

    const auto model {glm::translate(glm::mat4(1.0f), position)};
    mainShader->setMat4("model", model);

    glUniform1iv(glGetUniformLocation(mainShader->ID, "u_FaceLayers"), 6, blockType.faceLayers.data());

    glBindVertexArray(meshdata::MeshData::blockVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Rendering::gameRender(ShaderManager& shaderManager, GLFWwindow* window) {

    int width, height;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (const auto* mainShader = shaderManager.get("main")) {
        mainShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(texture::LoadTexture::block.atlas));
        mainShader->setInt("atlas", 0);

        glfwGetFramebufferSize(window, &width, &height);

        const glm::mat4 projection {glm::perspective(glm::radians(input::Player::getTargetFov()),
                        static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f)};
        mainShader->setMat4("projection", projection);

        // constexpr auto cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
        constexpr auto upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 cameraPos   = config::LevelData::get().getCameraPos();
        const glm::vec3 cameraView = input::Camera::getCameraView();

        m_View = glm::lookAt(cameraPos, cameraPos + cameraView, upVector);
        mainShader->setMat4("view", m_View);

        drawBlock(blockregistry::GRASS, glm::vec3(0.0f, 0.0f, -2.0f), shaderManager);

        drawBlock(blockregistry::DIRT, glm::vec3(1.2f, 0.0f, -2.0f), shaderManager);

        drawBlock(blockregistry::STONE, glm::vec3(2.4f, 0.0f, -2.0f), shaderManager);
    }

    if (const auto* crosshairShader = shaderManager.get("crosshair")) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_ONE, GL_ONE);
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
} // engine::rendering