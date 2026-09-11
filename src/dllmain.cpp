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

// Ждёт нужную страницу меню (максимум ~15 сек), возвращает true если дождались
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

    if (!WaitForMenuPage(MENUPAGE_MAIN_MENU, 15000)) {
        Log("Timeout waiting for Main Menu page (34). Aborting skip.");
        if (g_log) fclose(g_log);
        return 0;
    }
    Log("Main Menu detected. Forcing entry 0 (Start Game) + Enter.");

    __try { *g_currentMenuEntry = 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH writing menu entry (page 1)"); }

    PressEnter();

    if (!WaitForMenuPage(MENUPAGE_NEW_GAME, 5000)) {
        Log("Timeout waiting for Game submenu page (1). Aborting skip.");
        if (g_log) fclose(g_log);
        return 0;
    }
    Log("Game submenu detected. Forcing entry 0 (New Game) + Enter.");

    __try { *g_currentMenuEntry = 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH writing menu entry (page 2)"); }

    PressEnter();

    // логируем переход состояния для подтверждения
    int lastValue = -999999;
    for (int i = 0; i < 200; i++) {
        __try {
            int current = *gGameState;
            if (current != lastValue) {
                char buf[64];
                sprintf(buf, "gGameState: %d -> %d", lastValue, current);
                Log(buf);
                lastValue = current;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("CRASH reading gGameState"); break; }
        Sleep(50);
    }

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
