#include "SettingsManager.h"
#include <iostream>
#include <fstream>
#include <random>

#include "../ui/UIManager.h"
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace engine::config {

    SettingsManager* SettingsManager::s_Instance = nullptr;
    LevelData* LevelData::s_Instance = nullptr;

void SettingsManager::load() {
    if (!fs::exists(m_Filename)) {
        save();
        return;
    }

    try {
        std::ifstream inFile(m_Filename);
        json data;
        inFile >> data;

        m_Sensitivity    = data.value("sensitivity", 2.0f);
        m_SoundEffects   = data.value("soundEffects", 100.0f);
        m_MusicVolume    = data.value("musicVolume", 100.0f);
        m_BaseFov        = data.value("baseFov", 70.0f);
        m_RenderDistance = data.value("renderDistance", 12);
        m_FullscreenBool = data.value("fullscreen", false);

    } catch (const std::exception& e) {
        std::cerr << "JSON Load Error: " << e.what() << std::endl;
    }
}

void SettingsManager::save() {
    if (const fs::path p(m_Filename); p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }

    json data;
    data["sensitivity"]   = m_Sensitivity;
    data["soundEffects"]  = m_SoundEffects;
    data["musicVolume"]   = m_MusicVolume;
    data["baseFov"]       = m_BaseFov;
    data["renderDistance"]= m_RenderDistance;
    data["fullscreen"]    = m_FullscreenBool;

    if (std::ofstream outFile(m_Filename); outFile.is_open()) {
        outFile << data.dump(4);
        outFile.close();
    }
}

fs::path SettingsManager::getSaveDirectory() {
    fs::path baseDir;
#if defined(_WIN32)
    // Windows
    if (const char* localAppDataPath {std::getenv("LOCALAPPDATA")}) {
        baseDir = fs::path(localAppDataPath);
    } else {
        baseDir = fs::current_path();
    }
#elif defined(__linux__)
    // Linux
    const char* xdgDataPath = std::getenv("XDG_DATA_HOME");

    if (xdgDataPath && xdgDataPath[0] != '\0') {
        baseDir = fs::path(xdgDataPath);
    } else {
        const char* homePath = std::getenv("HOME");
        if (homePath) {
            baseDir = fs::path(homePath) / ".local" / "share";
        } else {
            baseDir = fs::current_path();
        }
    }
#else
    // Other weird Os' (like Mac)
    baseDir = fs::current_path();
#endif

    fs::path saveDir {baseDir / "Alex's_Minecraft_Recreation"};

    if (!fs::exists(saveDir)) {
        fs::create_directories(saveDir);
    }

    return saveDir;
}

long long generateSeed() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution dist(
        std::numeric_limits<long long>::min(),
        std::numeric_limits<long long>::max()
    );
    return dist(gen);
}

void LevelData::saveLevel() {
    if (m_CurrentWorldName.empty()) return;

    const std::string levelFilename {(SettingsManager::getSaveDirectory() / "saves" / m_CurrentWorldName /
        "level.json").string()};

    fs::create_directories(SettingsManager::getSaveDirectory() / "saves" / m_CurrentWorldName);

    json data;
    data["seed"] = m_Seed;
    data["playerX"] = m_CameraPos.x;
    data["playerY"] = m_CameraPos.y;
    data["playerZ"] = m_CameraPos.z;
    data["lastPlayed"] = saveTime();
    data["creativeMode"] = m_CreativeMode;


    if (std::ofstream outFile(levelFilename); outFile.is_open()) {
        outFile << data.dump(4);
        outFile.close();
    }
}

void LevelData::loadLevel() {
    if (m_CurrentWorldName.empty()) return;
    const std::string levelFilename {(SettingsManager::getSaveDirectory() / "saves" / m_CurrentWorldName /
        "level.json").string()};

    if (!fs::exists(levelFilename)) {
        if (!ui::UIManager::get().getSeedInput()) {
            m_Seed = generateSeed();
        }
        saveLevel();
        return;
    }

    try {
        std::ifstream inFile(levelFilename);
        json data;
        inFile >> data;
        m_Seed = data.value("seed", 0LL);

        m_LastPlayed = data.value("lastPlayed", "(Unknown)");

        m_CreativeMode = data.value("creativeMode", false);

        m_HasPlayerPos = data.contains("playerX");

        if (data.contains("playerX")) {
            m_CameraPos.x = data.value("playerX", 8.5f);
            m_CameraPos.y = data.value("playerY", 100.0f);
            m_CameraPos.z = data.value("playerZ", 8.5f);
        }

    } catch (const std::exception& e) {
        std::cerr << "JSON Load Error: " << e.what() << std::endl;
        m_Seed = generateSeed();
    }
}

std::string LevelData::saveTime() {
    const std::time_t now {std::time(nullptr)};

    if (const std::tm* localTime {std::localtime(&now)}; localTime != nullptr) {
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "(%d/%m/%Y %I:%M %p)", localTime);

        return (std::string{buffer});
    }

    return "(Time Error)";
}

} // engine::config
