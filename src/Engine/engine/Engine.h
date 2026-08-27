#ifndef MINECRAFT_RECREATION_RECREATION_ENGINE_H
#define MINECRAFT_RECREATION_RECREATION_ENGINE_H
#include <glad/gl.h>
#include "Engine/config/SettingsManager.h"
#include "Engine/window/WindowManager.h"
#include "Engine/sound/Sound.h"
#include "Engine/rendering/Rendering.h"
using namespace engine;

namespace engine {

class Engine {
    window::WindowManager m_WindowInstance;
    config::SettingsManager m_Settings;
    sound::Sound m_Sound;
    rendering::Rendering m_Rendering{};
    std::unique_ptr<rendering::ShaderManager> m_ShaderManager;
    std::chrono::steady_clock::time_point m_LastFrameTime;

    void initSystems();
    void gameLoop();
    static void shutdownSystems();

public:
    Engine();
    void run();
};

} // engine

#endif //MINECRAFT_RECREATION_RECREATION_ENGINE_H
