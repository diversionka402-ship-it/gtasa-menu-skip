#include <windows.h>
#include <cstdio>

// Известный (для версий 1.0 US и её репаков) адрес переменной состояния игры
volatile int* gGameState = reinterpret_cast<volatile int*>(0xC8D4C0);

DWORD WINAPI MonitorThread(LPVOID) {
    Sleep(3000); // даём игре время на инициализацию

    FILE* f = fopen("MenuSkip.log", "w");
    if (!f) return 0;
    fprintf(f, "Monitoring gGameState at 0xC8D4C0...\n");
    fflush(f);

    int lastValue = -999999;
    for (int i = 0; i < 600; i++) { // следим 60 секунд (600 * 100мс)
        __try {
            int current = *gGameState;
            if (current != lastValue) {
                fprintf(f, "[t=%d ms] gGameState changed: %d -> %d\n",
                        i * 100, lastValue, current);
                fflush(f);
                lastValue = current;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            fprintf(f, "[t=%d ms] READ CRASHED - address invalid for this exe!\n", i * 100);
            fflush(f);
            break;
        }
        Sleep(100);
    }

    fprintf(f, "Monitoring finished.\n");
    fclose(f);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MonitorThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
