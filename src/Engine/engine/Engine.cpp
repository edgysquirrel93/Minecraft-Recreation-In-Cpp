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
    glfwSetCursorPosCallback(m_WindowInstance.getWindow(), input::Camera::mouseCallback);
    m_ShaderManager = std::make_unique<rendering::ShaderManager>();
    m_Sound.init();
    meshdata::MeshData::Init();
    config::LevelData::get().setWorld(m_Rendering.getWorld());
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

        sound::Sound::get().update(deltaTime);

        ui::UIManager::update();

        if (ui::UIManager::getCurrentScreen() == ui::ScreenState::InGame)
        {
            const float frameTime {std::min(deltaTime, 0.1f)};
            input::Input::processInput(window);
            input::Player::processMovement(window, frameTime);
            m_Rendering.getWorld().update(config::LevelData::get().getCameraPos());
            glEnable(GL_DEPTH_TEST);
            m_Rendering.gameRender(*m_ShaderManager, window);
        }

        glDisable(GL_DEPTH_TEST);

        ui::UIManager::render();

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