#include "game.h"
#include <cstdio>
#pragma comment(lib, "user32.lib")

constexpr DWORD MENU_MANAGER_BASE = 0xBA6748;
static volatile char* g_currentMenuPage  = reinterpret_cast<volatile char*>(MENU_MANAGER_BASE + 0x15D);
static volatile int*  g_currentMenuEntry = reinterpret_cast<volatile int*>(MENU_MANAGER_BASE + 0x54);
volatile int* gGameState = reinterpret_cast<volatile int*>(0xC8D4C0);

constexpr char MENUPAGE_MAIN_MENU = 34;
constexpr char MENUPAGE_NEW_GAME  = 1;

static FILE* g_log = nullptr;
static void Log(const char* msg) { if (g_log) { fprintf(g_log, "%s\n", msg); fflush(g_log); } }

static void PressEnter() {
    INPUT input[2] = {};
    input[0].type = INPUT_KEYBOARD; input[0].ki.wVk = VK_RETURN;
    input[1].type = INPUT_KEYBOARD; input[1].ki.wVk = VK_RETURN; input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input[0], sizeof(INPUT));
    Sleep(20);
    SendInput(1, &input[1], sizeof(INPUT));
}

static bool WaitForMenuPage(char targetPage, int timeoutMs) {
    for (int waited = 0; waited < timeoutMs; waited += 10) {
        __try { if (*g_currentMenuPage == targetPage) return true; }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH reading m_nCurrentMenuPage"); return false; }
        Sleep(10);
    }
    return false;
}

void StartNewGameSequence() {
    g_log = fopen("MenuSkip.log", "w");
    Log("StartNewGameSequence begin");

    // на момент клика PLAY стандартное меню уже открыто (просто скрыто нашим UI)
    if (!WaitForMenuPage(MENUPAGE_MAIN_MENU, 3000)) Log("Main menu page not confirmed (continuing anyway)");

    __try { *g_currentMenuEntry = 0; } __except (EXCEPTION_EXECUTE_HANDLER) {}
    PressEnter();

    if (!WaitForMenuPage(MENUPAGE_NEW_GAME, 5000)) {
        Log("Game submenu timeout");
        if (g_log) fclose(g_log);
        return;
    }
    Sleep(300);

    int baseline = -999999;
    __try { baseline = *gGameState; } __except (EXCEPTION_EXECUTE_HANDLER) {}

    for (int attempt = 1; attempt <= 8; attempt++) {
        __try { *g_currentMenuEntry = 0; } __except (EXCEPTION_EXECUTE_HANDLER) {}
        PressEnter();
        Sleep(250);
        __try {
            if (*gGameState != baseline) { Log("New game triggered"); break; }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { break; }
    }

    Log("StartNewGameSequence done");
    if (g_log) fclose(g_log);
}
