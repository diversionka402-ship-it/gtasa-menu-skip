#include <windows.h>
#include <cstdio>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        FILE* f = fopen("MenuSkip.log", "w");
        if (f) {
            fprintf(f, "MenuSkip.asi loaded successfully!\n");
            fclose(f);
        }
    }
    return TRUE;
}
