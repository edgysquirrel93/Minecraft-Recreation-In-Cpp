#ifndef MINECRAFT_RECREATION_RECREATION_ENGINE_H
#define MINECRAFT_RECREATION_RECREATION_ENGINE_H
#include "../config/SettingsManager.h"
#include "../window/WindowManager.h"
using namespace engine;

namespace engine {

class Engine {
    window::WindowManager m_WindowInstance;
    config::SettingsManager m_Settings;

    void initSystems();
    void gameLoop() const;
    static void shutdownSystems();

public:
    Engine();
    void run();
};

} // engine

#endif //MINCRAFT_RECREATION_RECREATION_ENGINE_H
