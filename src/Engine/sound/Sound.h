#ifndef MINECRAFT_RECREATION_RECREATION_SOUND_H
#define MINECRAFT_RECREATION_RECREATION_SOUND_H
#include <miniaudio.h>
#include <vector>
#include <nlohmann/json.hpp>

namespace engine::sound {

class Sound {
    static Sound* s_Instance;
    bool m_Initialised;

    ma_sound_group m_MusicGroup{}, m_SFXGroup{};
    ma_engine m_AudioEngine{};
    ma_sound m_ClickSound{};

    std::unordered_map<std::string, std::vector<std::string>> m_Playlists;
    std::vector<std::string> m_CurrentActiveTracks;

    ma_sound m_CurrentMusic;
    bool m_IsMusicPlaying;
    bool m_IsWaitingForNextSong;
    float m_SilenceTimer;

    std::vector<std::string> m_CurrentPlaylist;
    int m_CurrentIndex;

    void playNextTrack();
public:
    Sound();
    ~Sound();

    static Sound& get() { return *s_Instance; }
    ma_engine* getEngine() { return &m_AudioEngine; }
    ma_sound_group* getMusicGroup() { return &m_MusicGroup; }
    ma_sound_group* getSFXGroup() { return &m_SFXGroup; }

    void playClickSound() { ma_sound_seek_to_pcm_frame(&m_ClickSound, 0); ma_sound_start(&m_ClickSound); }

    void init();
    static void playlistGen();
    void playPlaylist(const std::string& playlist);
    void update(float deltaTime);
    void stopAll();
    void shutdown();
};

}

#endif