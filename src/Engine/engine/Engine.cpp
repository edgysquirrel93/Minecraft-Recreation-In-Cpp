#include "../texture/Texture.h"
#include "../ui/UIManager.h"
#include "Engine.h"

namespace engine {

    Engine::Engine() : m_Settings("config.json") {}

    void Engine::initSystems() {
        m_Settings.load();
        m_WindowInstance.initWindow();
        m_Sound.init();
        texture::LoadTexture::loadAllTextures();
        ui::UIManager::init(m_WindowInstance.getWindow());
        m_LastFrameTime = std::chrono::steady_clock::now();
    }

    void Engine::gameLoop()
    {
        GLFWwindow* window = m_WindowInstance.getWindow();

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            auto currentFrameTime = std::chrono::steady_clock::now();

            const float deltaTime = std::chrono::duration<float>(currentFrameTime - m_LastFrameTime).count();

            m_LastFrameTime = currentFrameTime;

            ui::UIManager::update();
            ui::UIManager::render();

            sound::Sound::get().update(deltaTime);

            glfwSwapBuffers(window);
        }
    }

    void Engine::shutdownSystems() {
        ui::UIManager::shutdown();
        sound::Sound::get().shutdown();
    }

    void Engine::run() {
        initSystems();
        gameLoop();
        shutdownSystems();
    }

} // engine