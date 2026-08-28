#ifndef MINECRAFT_RECREATION_RECREATION_UIMANAGER_H
#define MINECRAFT_RECREATION_RECREATION_UIMANAGER_H

#include <bitset>
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
    PauseMenuScreen,
};

// enum class ModalScreen {
//     None,
//     PauseMenu,
//     Inventory,
//     Chest
// };

enum OverlayFlags {
    DebugScreen,
    Overlay_Count
};

class UIManager {
    static UIManager* s_Instance;
    inline static std::bitset<Overlay_Count> s_ActiveOverlays;
    // private variables
    inline static ImFont* s_McFont {nullptr};
    inline static GLFWwindow* s_GlfwWindow {nullptr};

    inline static auto s_CurrentScreen{ScreenState::MainMenu};
    inline static auto s_LastScreen{ScreenState::MainMenu};
    inline static std::string s_SelectedWorld;
    inline static bool s_RenameInitReq;
    inline static std::string s_SeedStringInput;
    inline static bool s_SeedInput {false};

    // UI Sizes
    inline static float s_Scale {1.0f};
    inline static float s_ButtonHeight {55.0f};

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
    static void drawDebugMenuScreen();

    public:
    UIManager();
    ~UIManager();

    static UIManager& get() { return *s_Instance; }

    static void init(GLFWwindow* window);
    static void update();
    static void render();
    static void shutdown();

    // setters/getters
    static bool getSeedInput() {return s_SeedInput;}
    static void setSeedInput(const bool s) {s_SeedInput = s;}
    static ScreenState getCurrentScreen() {return s_CurrentScreen;}
    static void setCurrentScreen(const ScreenState c) {s_CurrentScreen = c;}
    static ScreenState getLastScreen() {return s_LastScreen;}
    static void setLastScreen(const ScreenState l) {s_LastScreen = l;}
    static void toggleOverlay(const OverlayFlags flag) { s_ActiveOverlays.flip(flag); }
    static void disableOverlay(const OverlayFlags flag) { s_ActiveOverlays.set(flag, false);}
    static bool isOverlayActive(const OverlayFlags flag) { return s_ActiveOverlays.test(flag); }
};

} // engine::ui

#endif
