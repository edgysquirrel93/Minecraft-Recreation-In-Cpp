#ifndef MINCRAFT_RECREATION_RECREATION_UIMANAGER_H
#define MINCRAFT_RECREATION_RECREATION_UIMANAGER_H

#include <GLFW/include/GLFW/glfw3.h>

#include "imgui/imgui.h"

namespace engine::ui {

enum class ScreenState {
    MainMenu,
    BackgroundScreen,
    InMenu,
    InGame,
    SingleplayerScreen,
    OptionsScreen
};

class UIManager {
    static UIManager* s_Instance;
    // private variables
    inline static ImFont* s_McFont {nullptr};
    inline static GLFWwindow* s_GlfwWindow {nullptr};

    inline static auto s_CurrentScreen{ScreenState::MainMenu};
    bool m_SeedInput {false};

    // private functions
    static ImGuiIO& getIO() {return ImGui::GetIO();}
    static void drawMCText(const std::string& text, ImU32 col);
    static void drawMCText(const std::string& text, const ImVec4& col);
    static void drawMCText(const std::string& text);
    static void drawTextWithShadow(ImVec2 pos, const char* text, ImU32 col = IM_COL32_WHITE, const char* textEnd = nullptr);
    static bool minecraftButton(const char* label, ImVec2 size, bool disabled = false);

    // enum correlating functions
    static void drawMainMenu();
    static void drawBackgroundScreen();
    static void drawSingleplayerScreen();

    public:
    UIManager();
    ~UIManager();

    static UIManager& get() { return *s_Instance; }

    static void init(GLFWwindow* window);
    static void update();
    static void render();
    static void shutdown();

    // setters/getters
    [[nodiscard]] bool getSeedInput() const {return m_SeedInput;}
    void setSeedInput(const bool s) {m_SeedInput = s;}
};

} // engine::ui

#endif
