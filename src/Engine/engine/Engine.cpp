#include "Engine/texture/Texture.h"
#include "Engine/ui/UIManager.h"
#include "Engine/meshdata/MeshData.h"
#include "Engine/player/Input.h"
#include "Engine.h"

namespace engine {

    Engine::Engine() : m_Settings("config.json") {}

    void Engine::initSystems() {
        m_Settings.load();
        m_WindowInstance.initWindow();
        m_ShaderManager = std::make_unique<rendering::ShaderManager>();
        m_Sound.init();
        meshdata::MeshData::Init();
        texture::LoadTexture::loadAllTextures();
        ui::UIManager::init(m_WindowInstance.getWindow());
        m_LastFrameTime = std::chrono::steady_clock::now();
    }

    void Engine::gameLoop()
    {
        GLFWwindow* window = m_WindowInstance.getWindow();

        glEnable(GL_DEPTH_TEST);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            auto currentFrameTime = std::chrono::steady_clock::now();

            const float deltaTime = std::chrono::duration<float>(currentFrameTime - m_LastFrameTime).count();

            m_LastFrameTime = currentFrameTime;

            sound::Sound::get().update(deltaTime);

            ui::UIManager::update();
            ui::UIManager::render();

            if (ui::UIManager::getCurrentScreen() == ui::ScreenState::InGame)
            {
                input::Input::processInput(window);
                rendering::Rendering::gameRender(*m_ShaderManager, window);
            }

            m_WindowInstance.checkWindowState();

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