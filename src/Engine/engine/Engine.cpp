#include "../texture/Texture.h"
#include "../ui/UIManager.h"
#include "Engine.h"

namespace engine {

    Engine::Engine() : m_Settings("config.json") {}

    void Engine::initSystems() {
        m_Settings.load();
        m_WindowInstance.initWindow();
        texture::LoadTexture::loadAllTextures();
        ui::UIManager::init(m_WindowInstance.getWindow());
    }

    void Engine::gameLoop() const {
        GLFWwindow* window = m_WindowInstance.getWindow();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ui::UIManager::update();
            ui::UIManager::render();

            glfwSwapBuffers(window);
        }
    }

    void Engine::shutdownSystems() {
        ui::UIManager::shutdown();
    }

    void Engine::run() {
        initSystems();
        gameLoop();
        shutdownSystems();
    }

} // engine