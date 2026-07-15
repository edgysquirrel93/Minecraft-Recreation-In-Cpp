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
            // ma_sound_seek_to_pcm_frame(&clickSound, 0);
            // ma_sound_start(&clickSound);
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

void UIManager::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO& io {ImGui::GetIO()};

    // ma_sound_group_init(&engine, 0, NULL, &groupSFX);
    // ma_sound_init_from_file(&engine, "Sounds/SoundEffects/Click.wav", MA_SOUND_FLAG_DECODE, &groupSFX, nullptr, &clickSound);

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
        case ScreenState::InGame:
        case ScreenState::InMenu:
        case ScreenState::OptionsScreen:
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

    if (minecraftButton("Singleplayer", ImVec2(buttonWidth, 50))) {
        s_CurrentScreen = ScreenState::SingleplayerScreen;
    }

    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    if (minecraftButton("Options", ImVec2(buttonWidth, 50))) {
        // currentScreen = ScreenState::OptionsScreen;
    }

    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    if (minecraftButton("Quit Game", ImVec2(buttonWidth, 50))) {
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

    const auto title {"Select World"};
    ImGui::SetCursorPosY(windowHeight * 0.05f);
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(title).x) * 0.5f);
    drawMCText(title);

    ImGui::SetCursorPosY(windowHeight * 0.15f);
    ImGui::SetCursorPosX(0);
    static float pendingScrollY {};
    static bool hasPendingScroll {false};
    static bool scrollDragging {false};

    if (hasPendingScroll) {
        ImGui::SetNextWindowScroll(ImVec2(-1.0f, pendingScrollY));
        hasPendingScroll = false;
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0);

    float scrollY, scrollMax;
    ImVec2 winPos, winSize;
    scrollMax = {ImGui::GetScrollMaxY()};

    if (ImGui::BeginChild("WorldList", ImVec2(windowWidth, windowHeight * 0.6f), true, ImGuiWindowFlags_NoScrollbar)) {
        winPos = ImGui::GetWindowPos();
        winSize = ImGui::GetWindowSize();
        scrollY = {ImGui::GetScrollY()};
        ImDrawList* dl {ImGui::GetWindowDrawList()};
        static std::vector<std::string> worldNames;
        static std::string selectedWorld;
        static bool confirmDelete {false};
        static float lastWorldListRefresh {};

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
            bool isSelected {(name == selectedWorld)};
            const float startY {ImGui::GetCursorPosY()};
            const float boxWidth {windowWidth * 0.5f};
            constexpr float boxHeight {115.0f};
            const float boxPosX {(windowWidth - boxWidth) * 0.5f};

            std::string worldTime {"(No Date)"};
            std::string worldGameMode;
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

                } catch (...) {
                    worldGameMode = "Unknown Mode";
                    worldTime = "(Broken Save)";
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
                selectedWorld = name;
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

    if (scrollMax > 0) {
        float sbWidth {16.0f};
        float sbX {(winSize.x / 2 + (winSize.x / 4) + 16)};
        ImDrawList* fg {ImGui::GetForegroundDrawList()};

        fg->AddImage(ImTextureID(static_cast<unsigned int>(texture::LoadTexture::ui.scroller_background)),
        ImVec2(sbX, winPos.y),
        ImVec2(sbX + sbWidth, winPos.y + winSize.y));

        float thumbHeight {ImMax((winSize.y / (scrollMax + winSize.y)) * winSize.y, 32.0f)};
        float scrollPct {ImSaturate(scrollY / scrollMax)};
        float thumbY {winPos.y + scrollPct * (winSize.y - thumbHeight)};

        fg->AddImage(reinterpret_cast<ImTextureID>(static_cast<unsigned int>(texture::LoadTexture::ui.scroller)),
        ImVec2(sbX, thumbY),
        ImVec2(sbX + sbWidth, thumbY + thumbHeight));

        ImVec2 thumbMin(sbX, thumbY);

        if (ImVec2 thumbMax(sbX + sbWidth, thumbY + thumbHeight); ImGui::IsMouseHoveringRect(thumbMin, thumbMax) && ImGui::IsMouseClicked(0))
            scrollDragging = true;
        if (!ImGui::IsMouseDown(0)) scrollDragging = false;

        if (scrollDragging) {
            float delta {ImGui::GetIO().MouseDelta.y};
            float scrollRatio {scrollMax / (winSize.y - thumbHeight)};
            pendingScrollY = ImClamp(scrollY + delta * scrollRatio, 0.0f, scrollMax);
            hasPendingScroll  = true;
        }
    }

    constexpr float btnW {400.0f};
    constexpr float spacing {20.0f};
    const float bottomY {windowHeight - 60.0f};

    // bool hasSelection {!selectedWorld.empty()};

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.5f) - btnW - spacing, (bottomY - 60)));
    if (minecraftButton("Play Selected World", ImVec2(btnW, 50)/*, !hasSelection*/)) {
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
    if (minecraftButton("Rename", /*!hasSelection,*/ImVec2(btnW * 0.475, 50))) {
        // renameBuffer = selectedWorld;
        // originalName = selectedWorld;
        // renameWorldScreen = true;
        // singleplayerScreen = false;
        // renameInitReq = true;
    }

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.5f) - btnW + (btnW / 2) - (spacing / 2), bottomY));
    if (minecraftButton("Delete", /*!hasSelection,*/ ImVec2(btnW * 0.475, 50))) {
        // confirmDelete = true;
        // singleplayerScreen = false;
    }

    ImGui::SetCursorPos(ImVec2((windowWidth * 0.505f) - spacing, bottomY));
    if (minecraftButton("Cancel", ImVec2(btnW, 50))) {
        s_CurrentScreen = ScreenState::MainMenu;
    }

    ImGui::SetCursorPos(ImVec2(windowWidth * 0.505f - spacing, (bottomY - 60)));
    if (minecraftButton("Create New World", ImVec2(btnW, 50))) {
        // createWorldScreen = true;
        // singleplayerScreen = false;
    }

    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
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
