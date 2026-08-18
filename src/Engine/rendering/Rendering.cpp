#include "Rendering.h"
#include "Engine/meshdata/MeshData.h"
#include "Engine/texture/Texture.h"
#include "Engine/window/WindowManager.h"
#include "Engine/config/SettingsManager.h"
#include <glm/gtc/matrix_transform.hpp>

namespace engine::rendering
{
void Rendering::gameRender(ShaderManager& shaderManager, GLFWwindow* window) {

    int width, height;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (const auto* mainShader = shaderManager.get("main")) {
        mainShader->use();
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture::LoadTexture::block.dirt));
        mainShader->setInt("dirt", 0);

        glfwGetFramebufferSize(window, &width, &height);

        const glm::mat4 projection = glm::perspective(glm::radians(config::SettingsManager::get().getBaseFov()),
                                                static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
        mainShader->setMat4("projection", projection);

        constexpr auto cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
        constexpr auto cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
        constexpr auto upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);
        mainShader->setMat4("view", view);
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
        constexpr float angle = 20.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        mainShader->setMat4("model", model);
        glBindVertexArray(meshdata::MeshData::blockVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    if (const auto* crosshairShader = shaderManager.get("crosshair")) {
        glDisable(GL_DEPTH_TEST);
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        float aspect = 1.0f;
        aspect = static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1);
        crosshairShader->use();
        crosshairShader->setFloat("aspectRatio", aspect);
        glBindVertexArray(meshdata::MeshData::crossVAO);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glEnable(GL_DEPTH_TEST);
    }
}
} // engine::rendering