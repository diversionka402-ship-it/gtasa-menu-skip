#include <windows.h>
#include <cstdio>

volatile int* gGameState = reinterpret_cast<volatile int*>(0xC8D4C0);

void PressEnter() {
    INPUT input[2] = {};

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_RETURN;

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = VK_RETURN;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &input[0], sizeof(INPUT)); // key down
    Sleep(80);
    SendInput(1, &input[1], sizeof(INPUT)); // key up
}

DWORD WINAPI SkipMenuThread(LPVOID) {
    FILE* f = fopen("MenuSkip.log", "w");
    if (f) { fprintf(f, "SkipMenuThread started\n"); fflush(f); }

    Sleep(5000); // ждём, пока догрузится главное меню (state 7)

    if (f) { fprintf(f, "[t=5000] Pressing Enter (Start Game)\n"); fflush(f); }
    PressEnter();

    Sleep(1500); // ждём, пока откроется подменю Game

    if (f) { fprintf(f, "[t=6500] Pressing Enter (New Game)\n"); fflush(f); }
    PressEnter();

    // логируем состояние ещё 20 секунд, чтобы убедиться что дошли до игры
    int lastValue = -999999;
    for (int i = 0; i < 200; i++) {
        __try {
            int current = *gGameState;
            if (current != lastValue) {
                if (f) { fprintf(f, "[t=%d ms] gGameState: %d -> %d\n", 6500 + i * 100, lastValue, current); fflush(f); }
                lastValue = current;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            if (f) { fprintf(f, "CRASH on read\n"); fflush(f); }
            break;
        }
        Sleep(100);
    }

    if (f) { fprintf(f, "Done.\n"); fclose(f); }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, SkipMenuThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
