#define MINIAUDIO_IMPLEMENTATION
#include "Sound.h"

#include <fstream>
#include <iostream>
#include <random>

#include "Engine/config/SettingsManager.h"

namespace engine::sound
{
Sound* Sound::s_Instance = nullptr;

Sound::Sound() :
      m_Initialised(),
      m_CurrentMusic(),
      m_IsMusicPlaying(),
      m_IsWaitingForNextSong(),
      m_SilenceTimer(),
      m_CurrentIndex()
    {
    s_Instance = this;
}

Sound::~Sound() {
    if (m_Initialised) {
        shutdown();
    }
    s_Instance = nullptr;
}

void Sound::playlistGen() {
    const std::filesystem::path saveDir = config::SettingsManager::getSaveDirectory();

    const std::filesystem::path playlistPath = saveDir / "playlist.json";

    if (std::filesystem::exists(playlistPath)) {return;}

    nlohmann::json playlists;

    playlists["menu"] = {
        "assets/Sound/Music/C418 - Floating Trees (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Moog City 2 (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Mutation (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Beginning 2 (Minecraft Volume Beta).wav"
    };

    playlists["survival"] = {
        "assets/Sound/Music/C418 - Wet Hands - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - The End (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Subwoofer Lullaby - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Moog City 2 (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Mutation (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Oxygène - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Mice on Venus - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Minecraft - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Floating Trees (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Haggstrom - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Key - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Living Mice - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Dry Hands - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418  - Sweden - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Beginning 2 (Minecraft Volume Beta).wav"
    };

    playlists["creative"] = {
        "assets/Sound/Music/C418 - Wet Hands - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - The End (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Taswell (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Moog City 2 (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Mutation (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Oxygène - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Subwoofer Lullaby - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Living Mice - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Mice on Venus - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Minecraft - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Haunt Muskie (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Key - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Dreiton (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Dry Hands - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Floating Trees (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Haggstrom - Minecraft Volume Alpha.wav",
        "assets/Sound/Music/C418 - Blind Spots (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Beginning 2 (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Biome Fest (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Aria Math (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418  - Sweden - Minecraft Volume Alpha.wav"
    };

    playlists["nether"] = {
        "assets/Sound/Music/C418 - Warmth (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Ballad of the Cats (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Concrete Halls (Minecraft Volume Beta).wav",
        "assets/Sound/Music/C418 - Dead Voxel (Minecraft Volume Beta).wav",
    };

    std::ofstream outFile(playlistPath);
    if (!outFile.is_open()) {
        std::cerr << "Failed to create default playlist.json at: " << playlistPath << std::endl;
        return;
    }

    outFile << playlists.dump(4);

    std::cout << "Generated default playlist.json" << std::endl;
}

void Sound::init() {
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.periodSizeInMilliseconds = 15;

    if (ma_engine_init(&engineConfig, &m_AudioEngine) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine!" << std::endl;
        return;
    }

    ma_sound_group_init(&m_AudioEngine, 0, nullptr, &m_MusicGroup);
    ma_sound_group_init(&m_AudioEngine, 0, nullptr, &m_SFXGroup);
    m_Initialised = true;

    ma_sound_init_from_file(&m_AudioEngine, "assets/Sound/SoundEffects/click.wav", MA_SOUND_FLAG_DECODE,
        &m_SFXGroup, nullptr, &m_ClickSound);

    if (config::SettingsManager::get().getMusicVolume() <= 0)
        ma_sound_group_set_volume(&m_MusicGroup, 0);
    else if (config::SettingsManager::get().getSoundEffects() <= 0)
        ma_sound_group_set_volume(&m_SFXGroup, 0);

    playlistGen();

    const std::filesystem::path playlistPath = config::SettingsManager::getSaveDirectory() / "playlist.json";
    if (std::ifstream file(playlistPath); file.is_open()) {
        nlohmann::json data;
        file >> data;
        for (auto& [category, paths] : data.items()) {
            m_Playlists[category] = paths.get<std::vector<std::string>>();
        }
    }

    playPlaylist("menu");

}

void Sound::playPlaylist(const std::string& playlist) {
    if (!m_Playlists.contains(playlist)) {
        std::cerr << "Playlist not found: " << playlist << std::endl;
        return;
    }

    stopAll();

    m_CurrentPlaylist = m_Playlists[playlist];

    std::random_device rd;
    std::mt19937 g(rd());
    std::ranges::shuffle(m_CurrentPlaylist, g);

    m_CurrentIndex = 0;
    playNextTrack();
}

void Sound::playNextTrack()
{
    if (m_CurrentPlaylist.empty()) return;

    stopAll();

    if (m_CurrentIndex >= m_CurrentPlaylist.size()) {
        m_CurrentIndex = 0;
    }

    const std::string& trackPath {m_CurrentPlaylist[m_CurrentIndex]};

    const ma_result result = ma_sound_init_from_file(
        &m_AudioEngine,
        trackPath.c_str(),
        MA_SOUND_FLAG_STREAM,
        &m_MusicGroup,
        nullptr,
        &m_CurrentMusic
    );

    if (result == MA_SUCCESS) {
        ma_sound_start(&m_CurrentMusic);
        m_IsMusicPlaying = true;
        m_CurrentIndex++;

        ma_sound_set_end_callback(&m_CurrentMusic, [](void* pUserData, ma_sound*) {
            auto* instance = static_cast<Sound*>(pUserData);
            instance->m_IsMusicPlaying = false;
        }, this);
    } else {
        std::cerr << "Failed to play music track: " << trackPath << std::endl;
        m_CurrentIndex++;
        m_IsMusicPlaying = false;
        m_IsWaitingForNextSong = true;
        m_SilenceTimer = 5.0f;
    }
}

void Sound::update(const float deltaTime) {
    if (!m_IsMusicPlaying && !m_IsWaitingForNextSong && !m_CurrentActiveTracks.empty()) {
        m_IsWaitingForNextSong = true;
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_real_distribution dis(600.0f, 1200.0f);

        m_SilenceTimer = dis(gen);

        return;
    }

    if (m_IsWaitingForNextSong) {
        m_SilenceTimer -= deltaTime;

        if (m_SilenceTimer <= 0.0f) {
            m_IsWaitingForNextSong = false;
            playNextTrack();
        }
    }
}

void Sound::stopAll() {
    if (m_IsMusicPlaying) {
        ma_sound_stop(&m_CurrentMusic);
        ma_sound_uninit(&m_CurrentMusic);
        m_IsMusicPlaying = false;
    }
}

void Sound::shutdown(){
    if (m_Initialised) {
        stopAll();
        ma_sound_group_uninit(&m_MusicGroup);
        ma_engine_uninit(&m_AudioEngine);
        m_Initialised = false;
    }
}

}