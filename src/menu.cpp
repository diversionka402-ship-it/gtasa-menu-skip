#include "imgui.h"
#include <windows.h>
#include "game.h"

static bool g_gameStarting = false;

static DWORD WINAPI StartGameThreadProc(LPVOID) {
    StartNewGameSequence();
    return 0;
}

void RenderMenu() {
    int state = 0;
    __try { state = *gGameState; } __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (state >= 9) return; // игра уже загрузилась - прячем свой интерфейс

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
    ImGui::Begin("##CustomMenu", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    ImGui::SetCursorPos(ImVec2(center.x - 150, center.y - 120));
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("MY MULTIPLAYER");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::SetCursorPos(ImVec2(center.x - 60, center.y));
    if (!g_gameStarting) {
        if (ImGui::Button("PLAY", ImVec2(120, 40))) {
            g_gameStarting = true;
            CreateThread(nullptr, 0, StartGameThreadProc, nullptr, 0, nullptr);
        }
    } else {
        ImGui::Text("Starting...");
    }

    ImGui::End();
    ImGui::PopStyleColor();
}
