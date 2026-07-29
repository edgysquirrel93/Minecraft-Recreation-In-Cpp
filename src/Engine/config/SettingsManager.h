#ifndef MINECRAFT_RECREATION_RECREATION_FILESYSTEM_H
#define MINECRAFT_RECREATION_RECREATION_FILESYSTEM_H
#include <glm/glm.hpp>

#include <nlohmann/json.hpp>

namespace engine::config {

class SettingsManager {
    static SettingsManager* s_Instance;
    std::string m_Filename;
    float m_Sensitivity {};
    float m_SoundEffects {};
    float m_MusicVolume {};
    float m_BaseFov {};
    int m_RenderDistance {};
    bool m_FullscreenBool {};
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
};

class LevelData {

    long long m_Seed = 0;
    std::string m_CurrentWorldName;
    glm::vec3 m_CameraPos{0.0f, 0.0f, 0.0f};
    bool m_HasPlayerPos = false;
    bool m_CreativeMode = false;
    std::string m_LastPlayed{};

    LevelData() = default;

public:

    static LevelData& get() {static LevelData instance; return instance;}

    LevelData(const LevelData&) = delete;
    LevelData& operator=(const LevelData&) = delete;

    void loadLevel();
    void saveLevel();
    static std::string saveTime();

    [[nodiscard]] long long getSeed() const { return m_Seed; }
    void setSeed(const long long s) { m_Seed = s; }
    [[nodiscard]] std::string getCurrentWorldName() const { return m_CurrentWorldName; }
    void setCurrentWorldName(const std::string& name) { m_CurrentWorldName = name; }
    [[nodiscard]] bool getCreativeModeBool() const { return m_CreativeMode; }
    void setCreativeModeBool(const bool f) {m_CreativeMode = f;}
};

} // engine::config

#endif
