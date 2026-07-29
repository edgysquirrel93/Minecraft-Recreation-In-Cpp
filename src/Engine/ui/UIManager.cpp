#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "../texture/Texture.h"
#include "UIManager.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "../config/SettingsManager.h"

namespace engine::ui {

UIManager* UIManager::s_Instance = nullptr;

UIManager::UIManager() {s_Instance = this;}

UIManager::~UIManager() {if (s_Instance == this) s_Instance = nullptr;}

void UIManager::drawTextWithShadow(const ImVec2 pos, const char* text, const ImU32 col, const char* textEnd) {
    ImDrawList* drawList {ImGui::GetWindowDrawList()};
    const ImFont* font {ImGui::GetFont()};
    const float fontSize {ImGui::GetFontSize()};

    const ImVec4 c {ImGui::ColorConvertU32ToFloat4(col)};
    const ImU32 shadowCol {ImGui::ColorConvertFloat4ToU32(ImVec4(c.x * 0.25f, c.y * 0.25f, c.z * 0.25f, c.w))};

    drawList->AddText(font, fontSize, ImVec2(pos.x + 2.5f, pos.y + 2.5f), shadowCol, text, textEnd);
    drawList->AddText(font, fontSize, pos, col, text, textEnd);
}

bool UIManager::minecraftButton(const char* label, const ImVec2 size, const bool disabled) {
    ImGuiWindow* window {ImGui::GetCurrentWindow()};
    if (window->SkipItems) return false;

    const ImGuiID id {window->GetID(label)};
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered {false}, held {false}, pressed {false};

    if (!disabled) {
        pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (pressed) {
            sound::Sound::get().playClickSound();
        }
    }

    ImDrawList* drawList {ImGui::GetWindowDrawList()};
    unsigned int currentTexId {};
    ImU32 textColor;

    if (disabled) {
        currentTexId = static_cast<unsigned int>(texture::LoadTexture::ui.button_disabled);
        textColor = IM_COL32(85, 85, 85, 255);
    } else if (hovered || held) {
        currentTexId = static_cast<unsigned int>(texture::LoadTexture::ui.button_highlighted);
        textColor = IM_COL32_WHITE;
    } else {
        currentTexId = static_cast<unsigned int>(texture::LoadTexture::ui.button);
        textColor = IM_COL32_WHITE;
    }

    drawList->AddImage(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(currentTexId)), bb.Min, bb.Max);

    const char* labelEnd {ImGui::FindRenderedTextEnd(label)};
    const ImVec2 textSize {ImGui::CalcTextSize(label, labelEnd)};
    const ImVec2 textPos {bb.Min + (size - textSize) * 0.5f};
    drawTextWithShadow(textPos, label, textColor, labelEnd);

    return pressed;
}

bool UIManager::minecraftSlider(const char* label, char textDisplay[], float* value, const float min, const float max) {
    ImGuiWindow* window {ImGui::GetCurrentWindow()};
    if (window->SkipItems) return false;

    ImGuiContext& g {*GImGui};
    const ImGuiID id {window->GetID(label)};

    constexpr ImVec2 size(500, 55);
    const ImVec2 pos {window->DC.CursorPos};
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    const float old_value = *value;

    static std::unordered_map<ImGuiID, float> smoothValues;
    static std::unordered_map<ImGuiID, bool> smoothInit;

    if (!smoothInit.contains(id))
    {
        smoothValues[id] = *value;
        smoothInit[id] = true;
    }
    float& smooth {smoothValues[id]};

    constexpr float thumbWidth {20.0f};
    const float trackUsable {bb.GetWidth() - thumbWidth};
    const float trackStartX {bb.Min.x + thumbWidth * 0.5f};

    auto getMouseX = [&]() -> float {
        double mx, my;
        glfwGetCursorPos(s_GlfwWindow, &mx, &my);
        return static_cast<float>(mx);
    };

    auto mousePosToValue = [&]() -> float {
        const float mx {getMouseX()};
        float t {(mx - trackStartX) / trackUsable};
        t = ImClamp(t, 0.0f, 1.0f);
        return min + t * (max - min);
    };

    const bool hovered {ImGui::IsMouseHoveringRect(bb.Min, bb.Max)};

    if (hovered && ImGui::IsMouseClicked(0))
    {
        if (g.ActiveId == 0 || g.ActiveId == id)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdMouseButton = 0;
            *value = mousePosToValue();
        }
    }

    if (g.ActiveId == id)
    {
        ImGui::KeepAliveID(id);
        g.ActiveIdMouseButton = 0;

        if (ImGui::IsMouseDown(0))
        {
            *value = mousePosToValue();
        }
        else
        {
            ImGui::ClearActiveID();
            sound::Sound::get().playClickSound();
        }
    }

    const bool value_changed {(*value != old_value)};
    const bool is_active {(g.ActiveId == id)};

    smooth = *value;

    ImDrawList* drawList {ImGui::GetWindowDrawList()};

    unsigned const int trackTexHi {texture::LoadTexture::ui.slider_highlighted},
    trackTex {texture::LoadTexture::ui.slider},
    thumbTexHi {texture::LoadTexture::ui.slider_handle_highlighted},
    thumbTex {texture::LoadTexture::ui.slider_handle};

    const unsigned int currentTrack {(hovered || is_active) ? trackTexHi : trackTex};
    drawList->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(currentTrack)), bb.Min, bb.Max);

    const float percentage {ImClamp((smooth - min) / (max - min), 0.0f, 1.0f)};
    const float thumbCenterX {trackStartX + percentage * trackUsable};
    const float thumbPosX {thumbCenterX - thumbWidth * 0.5f};

    const ImVec2 t_min(thumbPosX, bb.Min.y);
    const ImVec2 t_max(thumbPosX + thumbWidth, bb.Max.y);

    const bool thumbHovered {ImGui::IsMouseHoveringRect(t_min, t_max)};
    const unsigned int currentThumb {(is_active || thumbHovered) ? thumbTexHi : thumbTex};
    drawList->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(currentThumb)), t_min, t_max);

    const ImVec2 textSize = ImGui::CalcTextSize(textDisplay);
    const ImVec2 textPos(
        bb.Min.x + (bb.GetWidth() - textSize.x) * 0.5f,
        bb.Min.y + (bb.GetHeight() - textSize.y) * 0.5f
    );

    drawTextWithShadow(textPos, textDisplay);

    return value_changed;
}

bool UIManager::minecraftTextInput(const char* label, std::string& inputText, const ImVec2 size) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g {*GImGui};
    const ImGuiID id {window->GetID(label)};

    const ImVec2 pos {window->DC.CursorPos};
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    const bool hovered {ImGui::IsMouseHoveringRect(bb.Min, bb.Max)};
    const bool focused {(g.ActiveId == id)};

    if (hovered && ImGui::IsMouseClicked(0))
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
    }

    // Handle keyboard input when focused
    if (focused)
    {
        // Handle backspace
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !inputText.empty())
            inputText.pop_back();

        // Handle regular character input
        for (const ImWchar c : g.IO.InputQueueCharacters)
        {
            if (constexpr int maxChars {32}; c >= 32 && inputText.length() < maxChars)
                inputText += static_cast<char>(c);
        }
    }

    if (!hovered && ImGui::IsMouseClicked(0))
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    // Render
    ImDrawList* drawList {ImGui::GetWindowDrawList()};
    unsigned const int texHighlighted {texture::LoadTexture::ui.text_field_highlighted},
    texNormal {texture::LoadTexture::ui.text_field};
    const unsigned int currentTex {(hovered || focused) ? texHighlighted : texNormal};
    drawList->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(currentTex)), bb.Min, bb.Max);

    // Draw text
    std::string displayText {inputText};
    if (focused && static_cast<int>(ImGui::GetTime() * 2) % 2 == 0)
        displayText += "_";

    const ImVec2 textPos(bb.Min.x + 10.0f, bb.Min.y + (bb.GetHeight() - 32.0f) * 0.5f);
    drawTextWithShadow(textPos, displayText.c_str());

    return focused;
}

void UIManager::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO& io {ImGui::GetIO()};

    s_GlfwWindow = window;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // load font
    s_McFont = io.Fonts->AddFontFromFileTTF("assets/Fonts/Monocraft-ttf/Monocraft.ttf", 32.0f);
}

void UIManager::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("UI", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

    switch (s_CurrentScreen) {
        case ScreenState::MainMenu:
            drawMainMenu();
            drawBackgroundScreen();
            break;
        case ScreenState::SingleplayerScreen:
            drawSingleplayerScreen();
            drawBackgroundScreen();
            break;
        case ScreenState::OptionsScreen:
            drawOptionsScreen();
            drawBackgroundScreen();
            break;
        case ScreenState::DeleteWorldScreen:
            drawDeleteWorldScreen();
            drawBackgroundScreen();
            break;
        case ScreenState::RenameWorldScreen:
            drawRenameWorldScreen();
            drawBackgroundScreen();
            break;
        case ScreenState::CreateNewWorldScreen:
            drawCreateNewWorldScreen();
            drawBackgroundScreen();
            break;
        case ScreenState::MoreWorldOptionsScreen:
            drawMoreWorldOptionsScreen();
            drawBackgroundScreen();
            break;
        case ScreenState::InGame:
        case ScreenState::InMenu:
        case ScreenState::BackgroundScreen:
            break;
    }

    ImGui::End();
}

void UIManager::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::drawMainMenu() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getIO().DisplaySize);
    ImGui::Begin("MainMenu", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar);

    ImGui::PushFont(s_McFont);

    constexpr float logoWidth {960.0f};
    constexpr float logoHeight {540.0f};
    ImGui::SetCursorPosX((getIO().DisplaySize.x - logoWidth) * 0.5f);
    ImGui::SetCursorPosY(getIO().DisplaySize.y * 0.0025f);

    const GLuint rawTextureId {texture::LoadTexture::ui.logo};

    ImGui::Image(
        reinterpret_cast<ImTextureID>(static_cast<intptr_t>(rawTextureId)),
        ImVec2(logoWidth, logoHeight)
    );

    const float windowWidth {ImGui::GetWindowSize().x};
    constexpr float buttonWidth {600.0f};
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    ImGui::SetCursorPosY(getIO().DisplaySize.y * 0.5f);

    if (minecraftButton("Singleplayer", ImVec2(buttonWidth, 55))) {
        s_CurrentScreen = ScreenState::SingleplayerScreen;
    }

    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    if (minecraftButton("Options", ImVec2(buttonWidth, 55))) {
        s_CurrentScreen = ScreenState::OptionsScreen;
    }

    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    if (minecraftButton("Quit Game", ImVec2(buttonWidth, 55))) {
        glfwSetWindowShouldClose(s_GlfwWindow, true);
    }

    ImGui::PopFont();
    ImGui::End();
}

void UIManager::drawBackgroundScreen() {
    ImGui::SetNextWindowPos(ImVec2(-1.0f, -1.0f));
    ImGui::SetNextWindowSize(ImVec2(getIO().DisplaySize.x + 2.0f, getIO().DisplaySize.y + 2.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Background", nullptr,
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoInputs |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoBringToFrontOnFocus);

    const float tileCountX {getIO().DisplaySize.x / 128.0f};
    const float tileCountY {getIO().DisplaySize.y / 128.0f};

    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<unsigned int>(texture::LoadTexture::ui.dirt_ui)),
        ImVec2(getIO().DisplaySize.x + 2.0f, getIO().DisplaySize.y + 2.0f),
        ImVec2(0, 0), ImVec2(tileCountX, tileCountY),
        ImVec4(0.15f, 0.15f, 0.15f, 1.0f), ImVec4(0,0,0,0));

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void UIManager::drawSingleplayerScreen() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getIO().DisplaySize);
    ImGui::Begin("SingleplayerScreen", nullptr,
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

    ImGui::PushFont(s_McFont);
    const float windowWidth {ImGui::GetWindowSize().x};
    const float windowHeight {ImGui::GetWindowSize().y};

    ImGui::SetWindowFontScale(1.2f);
    const auto title {"Select World"};
    ImGui::SetCursorPosY(windowHeight * 0.07f);
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(title).x) * 0.5f);
    drawMCText(title);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::SetCursorPosY(windowHeight * 0.15f);
    ImGui::SetCursorPosX(0);
    static float pendingScrollY {};
    static bool hasPendingScroll {false};
    static bool scrollDragging {false};
    m_RenameInitReq = false;

    if (hasPendingScroll) {
        ImGui::SetNextWindowScroll(ImVec2(-1.0f, pendingScrollY));
        hasPendingScroll = false;
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0);

    float scrollY {}, scrollMax {};
    ImVec2 winPos, winSize;

    if (ImGui::BeginChild("WorldList", ImVec2(windowWidth, windowHeight * 0.6f), true, ImGuiWindowFlags_NoScrollbar)) {
        winPos = ImGui::GetWindowPos();
        winSize = ImGui::GetWindowSize();
        scrollY = {ImGui::GetScrollY()};
        scrollMax = {ImGui::GetScrollMaxY()};
        ImDrawList* dl {ImGui::GetWindowDrawList()};
        static std::vector<std::string> worldNames;
        static float lastWorldListRefresh {};

        if (glfwGetTime() - lastWorldListRefresh > 0.5f) {
            worldNames.clear();
            if (std::string savesPath {(config::SettingsManager::getSaveDirectory() / "saves").string()};
                std::filesystem::exists(savesPath)) {
                for (const auto& entry : std::filesystem::directory_iterator(savesPath))
                    if (entry.is_directory())
                        worldNames.push_back(entry.path().filename().string());
            }
            lastWorldListRefresh = static_cast<float>(glfwGetTime());
        }

        constexpr float dirtTileSize {128.0f};
        const ImU32 dirtTint {ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.0f))};
        const float uvV1 {scrollY / dirtTileSize};
        const float uvV2 {uvV1 + (winSize.y / dirtTileSize)};
        const float uvU2 {winSize.x / dirtTileSize};

        dl->AddImage(reinterpret_cast<ImTextureID>(static_cast<unsigned int>(texture::LoadTexture::ui.dirt_ui)),
                     ImVec2(winPos.x, winPos.y + scrollY),
                     ImVec2(winPos.x + winSize.x, winPos.y + winSize.y + scrollY),
                     ImVec2(0, uvV1), ImVec2(uvU2, uvV2), dirtTint);

        int i{};
        for (const auto& name : worldNames)
        {
            bool isSelected {(name == m_SelectedWorld)};
            const float startY {ImGui::GetCursorPosY()};
            const float boxWidth {windowWidth * 0.5f};
            constexpr float boxHeight {115.0f};
            const float boxPosX {(windowWidth - boxWidth) * 0.5f};

            std::string worldTime {"(No Date)"};
            std::string worldGameMode {"Unknown Mode"};
            std::string levelPath {(config::SettingsManager::getSaveDirectory() / "saves" / name /
                    "level.json").string()};

            if (std::filesystem::exists(levelPath)) {
                try {
                    std::ifstream inFile(levelPath);
                    nlohmann::json data;
                    inFile >> data;
                    bool isCreative {data.value("creativeMode", false)};
                    worldGameMode = isCreative ? "Creative Mode" : "Survival Mode";
                    worldTime = data.value("lastPlayed", "(New World)");

                } catch (const std::exception& e) {
                    // if getting the values fails
                    std::cerr << "An error occurred getting the level data" << e.what() << std::endl;
                }
            }

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 1));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 1));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 1));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
            }

            ImGui::SetCursorPosX(boxPosX);

            ImVec2 pMin {ImGui::GetCursorScreenPos()};
            auto pMax {ImVec2(pMin.x + boxWidth, pMin.y + boxHeight)};

            if (ImGui::Selectable(("##" + name).c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(boxWidth, 115))) {
                m_SelectedWorld = name;
            }

            ImDrawList* drawList {ImGui::GetWindowDrawList()};
            if (isSelected) {
                drawList->AddRect(pMin, pMax, IM_COL32(134, 132, 131, 255), 0.0f, 0, 2.0f);
            }
            ImGui::PopStyleColor(3);

            ImGui::SetCursorPosY(startY + 10.0f);
            ImGui::SetCursorPosX(boxPosX + 15.0f);
            drawMCText(name);

            ImGui::SetCursorPosY(startY + 45.0f);
            ImGui::SetCursorPosX(boxPosX + 15.0f);
            ImVec4 grey {ImGui::ColorConvertU32ToFloat4(IM_COL32(134, 132, 131, 255))};
            drawMCText(name + " " += worldTime, grey);

            ImGui::SetCursorPosY(startY + 75.0f);
            ImGui::SetCursorPosX(boxPosX + 15.0f);
            drawMCText(worldGameMode, grey);

            if (i < static_cast<int>(worldNames.size()) - 1) {
                ImGui::SetCursorPosY(startY + boxHeight + 10.0f);
            } else {
                ImGui::SetCursorPosY(startY + boxHeight);
            }
            i++;
        }


        float shadowH {30.0f};
        ImU32 c_black {IM_COL32(0, 0, 0, 255)};
        ImU32 c_trans {IM_COL32(0, 0, 0, 0)};

        dl->AddRectFilledMultiColor(
        ImVec2(winPos.x, winPos.y),
        ImVec2(winPos.x + winSize.x, winPos.y + shadowH),
        c_black, c_black, c_trans, c_trans);
        dl->AddRectFilledMultiColor(
        ImVec2(winPos.x, winPos.y + winSize.y - shadowH),
        ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
        c_trans, c_trans, c_black, c_black);

        ImGui::EndChild();
    }

    if (scrollMax > 0.0f) {
        float sbWidth {16.0f};

        float sbX {winPos.x + (winSize.x / 2.0f) + (winSize.x / 4.0f) + 16.0f};

        ImDrawList* fg {ImGui::GetForegroundDrawList()};

        auto bgTex = reinterpret_cast<ImTextureID>(static_cast<unsigned int>(texture::LoadTexture::ui.scroller_background));
        auto thumbTex = reinterpret_cast<ImTextureID>(static_cast<unsigned int>(texture::LoadTexture::ui.scroller));

        fg->AddImage(bgTex, ImVec2(sbX, winPos.y), ImVec2(sbX + sbWidth, winPos.y + winSize.y));

        float thumbHeight {ImMax((winSize.y / (scrollMax + winSize.y)) * winSize.y, 32.0f)};
        float scrollPct {ImSaturate(scrollY / scrollMax)};
        float thumbY {winPos.y + scrollPct * (winSize.y - thumbHeight)};

        fg->AddImage(thumbTex, ImVec2(sbX, thumbY), ImVec2(sbX + sbWidth, thumbY + thumbHeight));

        ImVec2 thumbMin(sbX, thumbY);

        if (ImVec2 thumbMax(sbX + sbWidth, thumbY + thumbHeight); ImGui::IsMouseHoveringRect(thumbMin, thumbMax) && ImGui::IsMouseClicked(0)) {
            scrollDragging = true;
        }
        if (!ImGui::IsMouseDown(0)) {
            scrollDragging = false;
        }

        if (scrollDragging) {
            float delta {ImGui::GetIO().MouseDelta.y};
            float scrollRatio {scrollMax / (winSize.y - thumbHeight)};
            pendingScrollY = ImClamp(scrollY + delta * scrollRatio, 0.0f, scrollMax);
            hasPendingScroll = true;
        }
    }

    constexpr float btnW {400.0f};
    constexpr float spacing {20.0f};
    const float bottomY {windowHeight - 60.0f};

    bool hasSelection {!m_SelectedWorld.empty()};

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.5f) - btnW - spacing, (bottomY - 60)));
    if (minecraftButton("Play Selected World", ImVec2(btnW, 55), !hasSelection)) {
        // if (creativeMode) {
        //     SoundClass::QueuePlaylist("Sounds/Creative Songs");
        // } else {
        //     SoundClass::QueuePlaylist("Sounds/Survival Songs");
        // }
        // currentState = LOADING;
        // currentWorldName = selectedWorld;
        // loadingScreen = true;
        // worldLoaded = false; // Ensure this is false so worker starts
        // isWorldReady = false;
        // singleplayerScreen = false;
        // loadingProgress = 0.0f;
        //
        // {
        //     std::lock_guard lock(queueMutex);
        //     worldMap.clear();
        //     pendingTasks.clear();
        //     finishedTasks.clear();
        // }
        //
        // std::string nameToLoad = selectedWorld;
        //
        // bool expected = false;
        // if (isLoading.compare_exchange_strong(expected, true)) {
        //     std::thread([nameToLoad]() {
        //         startWorldLoad(nameToLoad);
        //     }).detach();
        // }
    }

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.5f) - btnW - spacing, bottomY));
    if (minecraftButton("Rename", ImVec2(btnW * 0.475, 55), !hasSelection)) {
        m_RenameInitReq = true;
        s_CurrentScreen = ScreenState::RenameWorldScreen;
    }

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.5f) - btnW + (btnW / 2) - (spacing / 2), bottomY));
    if (minecraftButton("Delete", ImVec2(btnW * 0.475, 55), !hasSelection)) {
        s_CurrentScreen = ScreenState::DeleteWorldScreen;
    }

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.505f) - spacing, bottomY));
    if (minecraftButton("Cancel", ImVec2(btnW, 55))) {
        s_CurrentScreen = ScreenState::MainMenu;
    }

    ImGui::SetCursorPos(ImVec2(windowWidth * 0.505f - spacing, (bottomY - 60)));
    if (minecraftButton("Create New World", ImVec2(btnW, 55))) {
        config::LevelData::get().setCurrentWorldName("New World");
        s_CurrentScreen = ScreenState::CreateNewWorldScreen;
    }

    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}
void UIManager::drawOptionsScreen()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(getIO().DisplaySize, ImGuiCond_Always);
    ImGui::Begin("OptionsScreen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);

    ImGui::PushFont(s_McFont);
    ImGui::SetWindowFontScale(1.2f);

    const float windowWidth {ImGui::GetWindowSize().x};
    const float center {windowWidth * 0.5f};
    constexpr float buttonWidth {500.0f};
    constexpr float spacing {15.0f};

    ImGui::SetCursorPosY(getIO().DisplaySize.y * 0.3f);
    constexpr auto optTitle {"Options"};
    ImGui::SetCursorPosX(center - ImGui::CalcTextSize(optTitle).x * 0.5f);
    drawMCText(optTitle);

    ImGui::SetWindowFontScale(1.0f);

    char musicDisplay[64];
    sprintf(musicDisplay, "Music: %.0f%%", config::SettingsManager::get().getMusicVolume());

    ImGui::SetCursorPosY(getIO().DisplaySize.y * 0.45f);
    ImGui::SetCursorPosX(center - buttonWidth - spacing);

    float musicVolume {config::SettingsManager::get().getMusicVolume()};

    if (constexpr auto* musicLabel {"##music"}; minecraftSlider(musicLabel, musicDisplay,
        &musicVolume, 0.0f, 100.0f)) {
        config::SettingsManager::get().setMusicVolume(musicVolume);

        const float volumeNormalized {musicVolume / 100.0f};

        ma_sound_group_set_volume(sound::Sound::get().getMusicGroup(), volumeNormalized);
    }

    char soundDisplay[64];
    sprintf(soundDisplay, "Sound: %.0f%%", config::SettingsManager::get().getSoundEffects());

    ImGui::SameLine();
    ImGui::SetCursorPosX(center + spacing);

    float soundVolume {config::SettingsManager::get().getSoundEffects()};

    if (constexpr auto* soundLabel {"##sound"}; minecraftSlider(soundLabel, soundDisplay,
        &soundVolume, 0.0f, 100.0f)) {
        config::SettingsManager::get().setSoundEffects(soundVolume);

        const float volumeNormalized {soundVolume / 100.0f};

        ma_sound_group_set_volume(sound::Sound::get().getSFXGroup(), volumeNormalized);
        }

    float baseFov = config::SettingsManager::get().getBaseFov();

    char fovDisplay[64];
    if (baseFov >= 109.9f) sprintf(fovDisplay, "FOV: Quake Pro");
    else if (baseFov <= 30.1f) sprintf(fovDisplay, "FOV: Cinematic");
    else if (baseFov >= 69.5f && baseFov <= 70.5f) sprintf(fovDisplay, "FOV: Normal");
    else sprintf(fovDisplay, "FOV: %.0f", baseFov);

    ImGui::SetCursorPosX(center - buttonWidth - spacing);

    if (constexpr auto fovLabel {"##fov"};
        minecraftSlider(fovLabel, fovDisplay, &baseFov, 30.0f, 110.0f))
        config::SettingsManager::get().setBaseFov(baseFov);

    float sensitivity {config::SettingsManager::get().getSensitivity()};

    char sensDisplay[64];
    const float displayPercentage {(sensitivity / 2.0f) * 100.0f};

    if (sensitivity <= 0.011f) sprintf(sensDisplay, "Sensitivity: *yawn*");
    else if (sensitivity >= 3.99f) sprintf(sensDisplay, "Sensitivity: HYPERSPEED");
    else sprintf(sensDisplay, "Sensitivity: %.0f%%", displayPercentage);

    ImGui::SameLine();
    ImGui::SetCursorPosX(center + spacing);

    if (constexpr auto sensLabel {"##sens"};
        minecraftSlider(sensLabel, sensDisplay, &sensitivity, 0.01f, 4.0f))
        config::SettingsManager::get().setSensitivity(sensitivity);

    ImGui::SetCursorPosX(center + spacing);

    char renderDisDisplay[64];

    int renderDistance {config::SettingsManager::get().getRenderDistance()};

    renderDistance = renderDistance / 2 * 2;

    snprintf(renderDisDisplay, sizeof(renderDisDisplay), "Render Distance: %.0d", renderDistance);

    float tempRenderDist {static_cast<float>(renderDistance)};

    if (constexpr auto renderDistanceDisplay = "##renderDistance";
    minecraftSlider(renderDistanceDisplay, renderDisDisplay, &tempRenderDist, 2.0f, 24.0f)) {
        renderDistance = static_cast<int>(tempRenderDist);
        config::SettingsManager::get().setRenderDistance(renderDistance);
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(center - buttonWidth - spacing);

    bool fullscreenBool {config::SettingsManager::get().getFullscreenBool()};
    std::string fullscreenLabel;

    if (fullscreenBool == false)
        fullscreenLabel = "OFF";
    else
        fullscreenLabel = "ON";

    if (fullscreenBool == false && minecraftButton(("Fullscreen: " + fullscreenLabel).c_str(), ImVec2(400, 45))) {
        fullscreenLabel = "ON";
        fullscreenBool = true;
        config::SettingsManager::get().setFullscreenBool(fullscreenBool);
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(center - buttonWidth - spacing);

    if (fullscreenBool == true && minecraftButton(("Fullscreen: " + fullscreenLabel).c_str(), ImVec2(500, 55))) {
        fullscreenBool = false;
        config::SettingsManager::get().setFullscreenBool(fullscreenBool);
    }

    ImGui::SetCursorPosY(getIO().DisplaySize.y * 0.65f);
    ImGui::SetCursorPosX(center - 250.0f);

    if (minecraftButton("Done", ImVec2(500, 55))) {
        config::SettingsManager::get().save();
        s_CurrentScreen = ScreenState::MainMenu;
    }

    ImGui::PopFont();
    ImGui::End();
}

void UIManager::drawDeleteWorldScreen() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getIO().DisplaySize);
    ImGui::Begin("##ConfirmDeleteScreen", nullptr,
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

    const float windowWidth {ImGui::GetWindowSize().x};
    const float windowHeight {ImGui::GetWindowSize().y};

    ImGui::PushFont(s_McFont);

    const ImVec2 lineOne {ImGui::CalcTextSize("Are you sure you want to delete this world?")};
    const ImVec2 lineTwo {ImGui::CalcTextSize("'' Will be lost forever! (A long time!)")};

    const ImVec2 getSelectedWorldWidth = ImGui::CalcTextSize(m_SelectedWorld.c_str());
    ImGui::SetCursorPos(ImVec2((windowWidth / 2) - lineOne.x / 2, static_cast<float>(windowHeight * 0.4)));
    drawMCText("Are you sure you want to delete this world?");

    ImGui::SetCursorPos(ImVec2((windowWidth / 2) - (lineTwo.x + getSelectedWorldWidth.x) / 2, static_cast<float>(windowHeight * 0.45)));
    drawMCText("'" + m_SelectedWorld + "' Will be lost forever! (A long time!)");
    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - 515, static_cast<float>(windowHeight * 0.6)));

    if (minecraftButton("Delete##confirm", ImVec2(500, 55))) {
        std::filesystem::remove_all(config::SettingsManager::getSaveDirectory() / "saves" / m_SelectedWorld);
        m_SelectedWorld = "";
        s_CurrentScreen = ScreenState::SingleplayerScreen;
    }

    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(windowWidth / 2 + 15, static_cast<float>(windowHeight * 0.6)));

    if (minecraftButton("Cancel##confirm", ImVec2(500, 55))) {
        s_CurrentScreen = ScreenState::SingleplayerScreen;
    }
    ImGui::PopFont();
    ImGui::End();
}

void UIManager::drawRenameWorldScreen() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getIO().DisplaySize);
    ImGui::Begin("##RenameWorldScreen", nullptr,
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

    const float windowWidth {ImGui::GetWindowSize().x};
    const float windowHeight {ImGui::GetWindowSize().y};

    ImGui::PushFont(s_McFont);

    static std::string renameBuffer {m_SelectedWorld};
    static std::string originalName {m_SelectedWorld};

    if (m_RenameInitReq) {
        m_RenameInitReq = false;
        renameBuffer = m_SelectedWorld;
        originalName = m_SelectedWorld;
    }

    const ImVec2 pos = ImGui::CalcTextSize("Rename World");
    ImGui::SetCursorPos(ImVec2((windowWidth / 2) - (pos.x / 2), static_cast<float>(windowHeight * 0.1)));
    drawMCText("Rename World");

    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.365)));

    drawMCText("World Name", ImVec4(134, 132, 131, 255));

    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.4)));

    if (minecraftTextInput("##worldname", renameBuffer, ImVec2(windowWidth / 2, 55))) {}

    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.525)));
    if (minecraftButton("Rename##confirm", ImVec2(windowWidth / 2, 55))) {
        const std::string oldPath {(config::SettingsManager::getSaveDirectory() / "saves" / originalName).string()};
        const std::string newPath {(config::SettingsManager::getSaveDirectory() / "saves" / renameBuffer).string()};

        if (std::filesystem::exists(oldPath) && !renameBuffer.empty()) {
            std::filesystem::rename(oldPath, newPath);
        }

        m_SelectedWorld = "";
        originalName = "";
        renameBuffer = "";
        s_CurrentScreen = ScreenState::SingleplayerScreen;
    }

    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.6)));

    if (minecraftButton("Cancel##confirm", ImVec2(windowWidth / 2, 55))) {
        s_CurrentScreen = ScreenState::SingleplayerScreen;
    }

    ImGui::PopFont();
    ImGui::End();
}

void UIManager::drawCreateNewWorldScreen() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getIO().DisplaySize);
    ImGui::Begin("CreateWorldScreen", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar);

    ImGui::PushFont(s_McFont);

    const float windowWidth {ImGui::GetWindowSize().x};
    const float windowHeight {ImGui::GetWindowSize().y};

    const float centerX {windowWidth / 2.0f};
    constexpr float spacing {20.0f};

    ImGui::SetWindowFontScale(1.2f);
    const ImVec2 cNWSize {ImGui::CalcTextSize("Create New World")};
    ImGui::SetCursorPos(ImVec2(centerX - (cNWSize.x / 2), static_cast<float>(windowHeight * 0.07)));
    drawMCText("Create New World");
    ImGui::SetWindowFontScale(1.0f);

    std::string worldName {config::LevelData::get().getCurrentWorldName()};
    long long seed = config::LevelData::get().getSeed();
    bool creativeMode {config::LevelData::get().getCreativeModeBool()};
    const std::string savedInText {"Will be saved in: " + worldName};

    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.215)));
    drawMCText("World Name", ImVec4(134, 132, 131, 255));
    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.29) + 15));
    drawMCText(savedInText, ImVec4(134, 132, 131, 255));

    ImGui::SetCursorPosX(centerX - windowWidth / 2 / 2);
    ImGui::SetCursorPosY(static_cast<float>(windowHeight * 0.25));
    minecraftTextInput("##worldname", worldName, ImVec2(windowWidth / 2, 55));

    static std::string selectedGameMode {"Survival"};
    const std::string modeButtonText {"Game Mode: " + selectedGameMode};
    ImGui::SetCursorPos(ImVec2(static_cast<float>(centerX - (windowWidth / 3.25) / 2), static_cast<float>(windowHeight * 0.4)));
    if (selectedGameMode == "Survival" && minecraftButton((modeButtonText).c_str(), ImVec2(static_cast<float>(windowWidth / 3.25), 55))) {
        selectedGameMode = "Creative";
        creativeMode = true;
    }

    ImGui::SetCursorPos(ImVec2(static_cast<float>(centerX - (windowWidth / 3.25) / 2),
        static_cast<float>(windowHeight * 0.4)));
    if (selectedGameMode == "Creative" && minecraftButton((modeButtonText).c_str(), ImVec2(static_cast<float>(windowWidth / 3.25), 55))) {
        selectedGameMode = "Survival";
        creativeMode = false;
    }

    if (selectedGameMode == "Survival") {
        ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.45)));
        drawMCText("Search for resources, crafting, gain", ImVec4(134, 132, 131, 255));
        ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.4775)));
        drawMCText("levels, health and hunger", ImVec4(134, 132, 131, 255));
    }

    if (selectedGameMode == "Creative") {
        ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.45)));
        drawMCText("Unlimited Resources, free flying and", ImVec4(134, 132, 131, 255));
        ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.4775)));
        drawMCText("destroy blocks instantly", ImVec4(134, 132, 131, 255));
    }

    ImGui::SetCursorPos(ImVec2(static_cast<float>(centerX - windowWidth / 3.25 / 2), static_cast<float>(windowHeight * 0.7)));
    if (minecraftButton("More World Options...", ImVec2(static_cast<float>(windowWidth / 3.25), 55))) {
        s_CurrentScreen = ScreenState::MoreWorldOptionsScreen;
    }

    ImGui::SetCursorPos(ImVec2(centerX - (windowWidth / 3) - spacing, static_cast<float>(windowHeight * 0.9)));
    if (minecraftButton("Create World", ImVec2(windowWidth / 3, 55))) {

        if (m_SeedStringInput.empty()) {
            m_SeedInput = false;
        }

        else {
            m_SeedInput = true;

            try {
                size_t pos;
                seed = std::stoll(m_SeedStringInput, &pos);

                if (pos != m_SeedStringInput.length()) {
                    seed = static_cast<long long>(std::hash<std::string>{}(m_SeedStringInput));
                }
            }
            catch (...) {
                seed = static_cast<long long>(std::hash<std::string>{}(m_SeedStringInput));
            }
        }

        config::LevelData::get().setSeed(seed);
        config::LevelData::get().setCreativeModeBool(creativeMode);
        std::string finalName = worldName;

        if (std::filesystem::exists(config::SettingsManager::getSaveDirectory() / "saves" / finalName)) {
            int counter = 1;
            while (std::filesystem::exists(config::SettingsManager::getSaveDirectory() / "saves" /
                worldName / (" (" + std::to_string(counter) + ")"))) {
                counter++;
            }
            finalName = worldName + " (" + std::to_string(counter) + ")";
        }

        std::filesystem::create_directories(config::SettingsManager::getSaveDirectory() / "saves" / finalName);
        //
        // if (selectedGameMode == "Creative") {
        //     SoundClass::QueuePlaylist("Sounds/Creative Songs");
        // } else {
        //     SoundClass::QueuePlaylist("Sounds/Survival Songs");
        // }
        //
        worldName = finalName;
        m_SeedStringInput = "";
        config::LevelData::get().setCurrentWorldName(worldName);
        config::LevelData::get().saveLevel();
        // currentState = LOADING;
        // loadingScreen = true;
        // worldLoaded = false;
        // isWorldReady = false;
        // loadingProgress = 0.0f;
        //
        // createWorldScreen = false;
        //
        // {
        //     std::lock_guard lock(queueMutex);
        //     worldMap.clear();
        //     pendingTasks.clear();
        //     finishedTasks.clear();
        // }
        //
        // bool expected = false;
        // if (isLoading.compare_exchange_strong(expected, true)) {
        //     std::thread([finalName]() {
        //         startWorldLoad(finalName);
        //     }).detach();
        // }
    }
    ImGui::SetCursorPos(ImVec2(centerX + spacing, static_cast<float>(windowHeight * 0.9)));

    if (minecraftButton("Cancel", ImVec2(windowWidth / 3, 55))) {
        s_CurrentScreen = ScreenState::SingleplayerScreen;
        m_SeedStringInput = "";
        worldName = "New World";
    }

    config::LevelData::get().setCurrentWorldName(worldName);

    ImGui::PopFont();
    ImGui::End();
}

void UIManager::drawMoreWorldOptionsScreen() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getIO().DisplaySize);
    ImGui::Begin("MoreWorldOptionsScreen", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar);

    ImGui::PushFont(s_McFont);

    const float windowWidth {ImGui::GetWindowSize().x};
    const float windowHeight {ImGui::GetWindowSize().y};

    const float centerX {windowWidth / 2.0f};
    constexpr float spacing {20.0f};

    ImGui::SetWindowFontScale(1.2f);
    const ImVec2 cNWSize {ImGui::CalcTextSize("Create New World")};

    std::string worldName {config::LevelData::get().getCurrentWorldName()};
    long long seed = config::LevelData::get().getSeed();
    const bool creativeMode {config::LevelData::get().getCreativeModeBool()};

    ImGui::SetCursorPos(ImVec2(centerX - (cNWSize.x / 2), static_cast<float>(windowHeight * 0.07)));
    drawMCText("Create New World");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.215)));
    drawMCText("Seed for the World Generator", ImVec4(134, 132, 131, 255));
    ImGui::SetCursorPos(ImVec2(windowWidth / 2 - (windowWidth / 4), static_cast<float>(windowHeight * 0.29) + 15));
    drawMCText("Leave blank for a random seed", ImVec4(134, 132, 131, 255));

    ImGui::SetCursorPosX(centerX - (windowWidth / 2) / 2);
    ImGui::SetCursorPosY(static_cast<float>(windowHeight * 0.25));

    minecraftTextInput("##seed", m_SeedStringInput, ImVec2(windowWidth / 2, 55));

    ImGui::SetCursorPos(ImVec2(static_cast<float>(centerX - (windowWidth / 3.25) / 2), static_cast<float>(windowHeight * 0.7)));
    if (minecraftButton("Done", ImVec2(static_cast<float>(windowWidth / 3.25), 55))) {
        s_CurrentScreen = ScreenState::CreateNewWorldScreen;
    }

    ImGui::SetCursorPos(ImVec2(centerX - (windowWidth / 3) - spacing, static_cast<float>(windowHeight * 0.9)));
    if (minecraftButton("Create World", ImVec2(windowWidth / 3, 55))) {

        if (m_SeedStringInput.empty()) {
            m_SeedInput = false;
        }

        else {
            m_SeedInput = true;

            try {
                size_t pos;
                seed = std::stoll(m_SeedStringInput, &pos);

                if (pos != m_SeedStringInput.length()) {
                    seed = static_cast<long long>(std::hash<std::string>{}(m_SeedStringInput));
                }
            }
            catch (...) {
                seed = static_cast<long long>(std::hash<std::string>{}(m_SeedStringInput));
            }
        }

        config::LevelData::get().setSeed(seed);
        config::LevelData::get().setCreativeModeBool(creativeMode);
        std::string finalName = worldName;

        if (std::filesystem::exists(config::SettingsManager::getSaveDirectory() / "saves" / finalName)) {
            int counter = 1;
            while (std::filesystem::exists(config::SettingsManager::getSaveDirectory() / "saves" /
                worldName / (" (" + std::to_string(counter) + ")"))) {
                counter++;
            }
            finalName = worldName + " (" + std::to_string(counter) + ")";
        }

        std::filesystem::create_directories(config::SettingsManager::getSaveDirectory() / "saves" / finalName);
        //
        // if (selectedGameMode == "Creative") {
        //     SoundClass::QueuePlaylist("Sounds/Creative Songs");
        // } else {
        //     SoundClass::QueuePlaylist("Sounds/Survival Songs");
        // }
        //
        worldName = finalName;
        m_SeedStringInput = "";
        config::LevelData::get().setCurrentWorldName(worldName);
        config::LevelData::get().saveLevel();
        // currentState = LOADING;
        // loadingScreen = true;
        // worldLoaded = false;
        // isWorldReady = false;
        // loadingProgress = 0.0f;
        //
        // createWorldScreen = false;
        //
        // {
        //     std::lock_guard lock(queueMutex);
        //     worldMap.clear();
        //     pendingTasks.clear();
        //     finishedTasks.clear();
        // }
        //
        // bool expected = false;
        // if (isLoading.compare_exchange_strong(expected, true)) {
        //     std::thread([finalName]() {
        //         startWorldLoad(finalName);
        //     }).detach();
        // }
    }

    ImGui::SetCursorPos(ImVec2(centerX + spacing, static_cast<float>(windowHeight * 0.9)));
    if (minecraftButton("Cancel", ImVec2(windowWidth / 3, 55))) {
        s_CurrentScreen = ScreenState::SingleplayerScreen;
        m_SeedStringInput = "";
        worldName = "New World";
    }

    config::LevelData::get().setCurrentWorldName(worldName);

    ImGui::PopFont();
    ImGui::End();
}

void UIManager::drawMCText(const std::string& text, const ImU32 col) {
    drawTextWithShadow(ImGui::GetCursorScreenPos(), text.c_str(), col);
}

void UIManager::drawMCText(const std::string& text, const ImVec4& col) {
    const ImU32 packedCol {ImGui::ColorConvertFloat4ToU32(col)};
    drawMCText(text, packedCol);
}

void UIManager::drawMCText(const std::string& text) {
    drawMCText(text, IM_COL32_WHITE);
}

} // engine::ui
