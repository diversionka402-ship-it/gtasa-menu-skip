#include <windows.h>
#include <cstdio>
#pragma comment(lib, "user32.lib")

volatile int* gGameState = reinterpret_cast<volatile int*>(0xC8D4C0);

void PressKey(WORD vk) {
    INPUT input[2] = {};

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = vk;

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = vk;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &input[0], sizeof(INPUT));
    Sleep(80);
    SendInput(1, &input[1], sizeof(INPUT));
}

FILE* g_log = nullptr;

void Log(const char* msg) {
    if (g_log) { fprintf(g_log, "%s\n", msg); fflush(g_log); }
}

DWORD WINAPI SkipMenuThread(LPVOID) {
    g_log = fopen("MenuSkip.log", "w");
    Log("SkipMenuThread started");

    Sleep(5000); // ждём главное меню (state 7)

    Log("Pressing Up (Options -> Start Game)");
    PressKey(VK_UP);
    Sleep(500);

    Log("Pressing Enter (Start Game)");
    PressKey(VK_RETURN);

    // ждём подменю Game подольше, с запасом
    Sleep(2500);

    // Пытаемся нажать Enter (New Game) до 6 раз, пока состояние не изменится
    int baseline = *gGameState;
    char buf[128];
    sprintf(buf, "Baseline gGameState before New Game presses: %d", baseline);
    Log(buf);

    for (int attempt = 1; attempt <= 6; attempt++) {
        sprintf(buf, "Attempt %d: pressing Enter (New Game)", attempt);
        Log(buf);
        PressKey(VK_RETURN);
        Sleep(700);

        int current = *gGameState;
        if (current != baseline) {
            sprintf(buf, "State changed after attempt %d: %d -> %d", attempt, baseline, current);
            Log(buf);
            break;
        }
    }

    // логируем ещё 20 секунд все изменения состояния
    int lastValue = *gGameState;
    for (int i = 0; i < 200; i++) {
        __try {
            int current = *gGameState;
            if (current != lastValue) {
                sprintf(buf, "gGameState: %d -> %d", lastValue, current);
                Log(buf);
                lastValue = current;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("CRASH on read");
            break;
        }
        Sleep(100);
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
