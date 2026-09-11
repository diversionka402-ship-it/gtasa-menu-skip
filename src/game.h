#pragma once
#include <windows.h>

extern volatile int* gGameState;

void StartNewGameSequence(); // запускать в отдельном потоке
