// DllMain.cpp - Entry point of the injected DLL.

#include <Windows.h>

#include <cstdio>

#include "Addresses.h"
#include "Hook.h"
#include "Memory.h"

namespace {

// Waits for the game module to be available and records its base address.
DWORD WINAPI AttachThread(LPVOID parameter) {
    (void)parameter;

    // Give the game a moment to finish loading before we touch its memory.
    Sleep(1000);

    while (true) {
        HMODULE module = GetModuleHandleA("GTA_SA.EXE");
        if (module != nullptr) {
            sa::SetGameBase(reinterpret_cast<uintptr_t>(module));
            break;
        }
        Sleep(500);
    }

    // Install the D3D9 hooks once the game has created its device.
    while (!sa::InitializeOverlay()) {
        Sleep(1000);
    }

    return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    (void)reserved;

    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);

            HANDLE thread = CreateThread(nullptr, 0, AttachThread, nullptr, 0, nullptr);
            if (thread != nullptr) CloseHandle(thread);
            break;
        }

        case DLL_PROCESS_DETACH:
            sa::ShutdownOverlay();
            break;

        default:
            break;
    }

    return TRUE;
}
