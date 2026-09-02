#ifndef MINECRAFT_RECREATION_RECREATION_FILESYSTEM_H
#define MINECRAFT_RECREATION_RECREATION_FILESYSTEM_H
#include <glm/glm.hpp>

#include <nlohmann/json.hpp>

#include "Engine/worldgen/ChunkRendering.h"
#include "Engine/worldgen/WorldGen.h"

namespace engine::config {

    class World;

class SettingsManager {
    static SettingsManager* s_Instance;
    std::string m_Filename;
    float m_Sensitivity    {2.0f};
    float m_SoundEffects   {100.0f};
    float m_MusicVolume    {100.0f};
    float m_BaseFov        {70.0f};
    int   m_RenderDistance {12};
    bool  m_FullscreenBool {false};
    float m_GuiScale       {1.0f};

public:
    explicit SettingsManager(const std::string& path)
        : m_Filename((getSaveDirectory() / path).string()) { s_Instance = this; }

    ~SettingsManager() = default;

    static SettingsManager& get() {return *s_Instance;}

    static std::filesystem::path getSaveDirectory();

    void load();
    void save();

    // setters/getters
    [[nodiscard]] float getSensitivity() const {return m_Sensitivity;}
    void setSensitivity(const float s) {m_Sensitivity = s;}
    [[nodiscard]] float getSoundEffects() const {return m_SoundEffects;}
    void setSoundEffects(const float s) {m_SoundEffects = s;}
    [[nodiscard]] float getMusicVolume() const {return m_MusicVolume;}
    void setMusicVolume(const float m) {m_MusicVolume = m;}
    [[nodiscard]] float getBaseFov() const {return m_BaseFov;}
    void setBaseFov(const float b) {m_BaseFov = b;}
    [[nodiscard]] int getRenderDistance() const {return m_RenderDistance;}
    void setRenderDistance(const int r) {m_RenderDistance = r;}
    [[nodiscard]] bool getFullscreenBool() const {return m_FullscreenBool;}
    void setFullscreenBool(const bool f) {m_FullscreenBool = f;}
    [[nodiscard]] float getGuiScale() const {return m_GuiScale;}
    void setGuiScale (const float g) {m_GuiScale = g;}
};

class LevelData {

    long long m_Seed {0};
    std::string m_CurrentWorldName;
    glm::vec3 m_CameraPos{8.0f, 66.0f, 8.0f};
    bool m_HasPlayerPos {false};
    bool m_CreativeMode {false};
    std::string m_LastPlayed{};
    worldgen::World* m_World {nullptr};

    LevelData() = default;

public:

    static LevelData& get() {static LevelData instance; return instance;}

    LevelData(const LevelData&) = delete;
    LevelData& operator=(const LevelData&) = delete;

    void loadLevel();
    void saveLevel();
    static std::string saveTime();

    void setWorld(worldgen::World& world) { m_World = &world; }
    [[nodiscard]] worldgen::World* getWorld() { return m_World; }
    [[nodiscard]] const worldgen::World* getWorld() const { return m_World; }
    void clearWorld() { m_World = nullptr; }
    [[nodiscard]] long long getSeed() const { return m_Seed; }
    void setSeed(const long long s) { m_Seed = s; }
    [[nodiscard]] std::string getCurrentWorldName() const { return m_CurrentWorldName; }
    void setCurrentWorldName(const std::string& name) { m_CurrentWorldName = name; }
    [[nodiscard]] bool getCreativeModeBool() const { return m_CreativeMode; }
    void setCreativeModeBool(const bool c) {m_CreativeMode = c;}
    [[nodiscard]] glm::vec3 getCameraPos() const {return m_CameraPos;}
    void setCameraPos(const glm::vec3 c) {m_CameraPos = c;}
};

} // engine::config

#endif
