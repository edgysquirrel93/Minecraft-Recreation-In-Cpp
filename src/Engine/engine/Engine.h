#ifndef MINECRAFT_RECREATION_RECREATION_ENGINE_H
#define MINECRAFT_RECREATION_RECREATION_ENGINE_H
#include "../config/SettingsManager.h"
#include "../window/WindowManager.h"
#include "../sound/Sound.h"
using namespace engine;

namespace engine {

class Engine {
    window::WindowManager m_WindowInstance;
    config::SettingsManager m_Settings;
    sound::Sound m_Sound;
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
