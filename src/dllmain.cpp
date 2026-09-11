#include <windows.h>
#include <cstdio>
#pragma comment(lib, "user32.lib")

// FrontEndMenuManager instance (адрес известен для GTA SA 1.0 US / Compact)
constexpr DWORD MENU_MANAGER_BASE = 0xBA6748;
volatile char* g_currentMenuPage  = reinterpret_cast<volatile char*>(MENU_MANAGER_BASE + 0x15D); // m_nCurrentMenuPage
volatile int*  g_currentMenuEntry = reinterpret_cast<volatile int*>(MENU_MANAGER_BASE + 0x54);   // m_nCurrentMenuEntry
volatile int*  gGameState         = reinterpret_cast<volatile int*>(0xC8D4C0);

constexpr char MENUPAGE_MAIN_MENU = 34;
constexpr char MENUPAGE_NEW_GAME  = 1;

FILE* g_log = nullptr;
void Log(const char* msg) { if (g_log) { fprintf(g_log, "%s\n", msg); fflush(g_log); } }

void PressEnter() {
    INPUT input[2] = {};
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_RETURN;
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = VK_RETURN;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input[0], sizeof(INPUT));
    Sleep(20);
    SendInput(1, &input[1], sizeof(INPUT));
}

// Ждёт нужную страницу меню (максимум timeoutMs), возвращает true если дождались
bool WaitForMenuPage(char targetPage, int timeoutMs) {
    for (int waited = 0; waited < timeoutMs; waited += 10) {
        __try {
            if (*g_currentMenuPage == targetPage) return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("CRASH reading m_nCurrentMenuPage");
            return false;
        }
        Sleep(10);
    }
    return false;
}

DWORD WINAPI SkipMenuThread(LPVOID) {
    g_log = fopen("MenuSkip.log", "w");
    Log("SkipMenuThread started");

    // --- Экран 1: Main Menu -> Start Game ---
    if (!WaitForMenuPage(MENUPAGE_MAIN_MENU, 15000)) {
        Log("Timeout waiting for Main Menu page (34). Aborting skip.");
        if (g_log) fclose(g_log);
        return 0;
    }
    Log("Main Menu detected. Forcing entry 0 (Start Game) + Enter.");

    __try { *g_currentMenuEntry = 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH writing menu entry (page 1)"); }

    PressEnter();

    // --- Экран 2: Game submenu -> New Game ---
    if (!WaitForMenuPage(MENUPAGE_NEW_GAME, 5000)) {
        Log("Timeout waiting for Game submenu page (1). Aborting skip.");
        if (g_log) fclose(g_log);
        return 0;
    }
    Log("Game submenu detected.");

    Sleep(300); // даём анимации подменю доиграть

    int baseline = -999999;
    __try { baseline = *gGameState; }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

    char buf[96];
    sprintf(buf, "Baseline gGameState: %d", baseline);
    Log(buf);

    bool triggered = false;
    for (int attempt = 1; attempt <= 8 && !triggered; attempt++) {
        __try { *g_currentMenuEntry = 0; }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH writing menu entry"); }

        PressEnter();
        Sleep(250);

        __try {
            if (*gGameState != baseline) {
                sprintf(buf, "Attempt %d worked, gGameState now %d", attempt, *gGameState);
                Log(buf);
                triggered = true;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH reading gGameState"); break; }
    }

    if (!triggered) Log("New Game never triggered after 8 attempts.");

    Log("Done.");
    if (g_log) fclose(g_log);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, SkipMenuThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
