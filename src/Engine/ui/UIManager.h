#ifndef MINECRAFT_RECREATION_RECREATION_UIMANAGER_H
#define MINECRAFT_RECREATION_RECREATION_UIMANAGER_H

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "Engine/sound/Sound.h"

namespace engine::ui {

enum class ScreenState {
    MainMenu,
    InMenu,
    InGame,
    SingleplayerScreen,
    OptionsScreen,
    DeleteWorldScreen,
    RenameWorldScreen,
    CreateNewWorldScreen,
    MoreWorldOptionsScreen,
    PauseMenuScreen
};

class UIManager {
    static UIManager* s_Instance;
    // private variables
    inline static ImFont* s_McFont {nullptr};
    inline static GLFWwindow* s_GlfwWindow {nullptr};

    inline static auto s_CurrentScreen{ScreenState::MainMenu};
    inline static std::string m_SelectedWorld;
    inline static bool m_RenameInitReq;
    inline static std::string m_SeedStringInput;
    inline static bool m_SeedInput {false};

    // UI Sizes
    inline static float m_Scale {1.0f};
    inline static float m_ButtonHeight {55.0f};

    // private functions
    static ImGuiIO& getIO() {return ImGui::GetIO();}
    static void drawMCText(const std::string& text, ImU32 col);
    static void drawMCText(const std::string& text, const ImVec4& col);
    static void drawMCText(const std::string& text);
    static void drawTextWithShadow(ImVec2 pos, const char* text, ImU32 col = IM_COL32_WHITE, const char* textEnd = nullptr);
    static bool minecraftButton(const char* label, ImVec2 size, bool disabled = false);
    static bool minecraftSlider(const char* label, char textDisplay[], float* value, float min = 0.0f, float max = 1.0f);
    static bool minecraftTextInput(const char* label, std::string& inputText, ImVec2 size);

    // enum correlating functions
    static void drawMainMenu();
    static void drawBackgroundScreen();
    static void drawSingleplayerScreen();
    static void drawOptionsScreen();
    static void drawDeleteWorldScreen();
    static void drawRenameWorldScreen();
    static void drawCreateNewWorldScreen();
    static void drawMoreWorldOptionsScreen();
    static void drawPauseMenuScreen();

    public:
    UIManager();
    ~UIManager();

    static UIManager& get() { return *s_Instance; }

    static void init(GLFWwindow* window);
    static void update();
    static void render();
    static void shutdown();

    // setters/getters
    static bool getSeedInput() {return m_SeedInput;}
    static void setSeedInput(const bool s) {m_SeedInput = s;}
    static ScreenState getCurrentScreen() {return s_CurrentScreen;}
    static void setCurrentScreen(const ScreenState c) {s_CurrentScreen = c;}
};

} // engine::ui

#endif
