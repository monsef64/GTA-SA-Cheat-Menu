// Hook.h - D3D9 hooking and the ImGui overlay setup.

#pragma once

#include <Windows.h>

#include <d3d9.h>

namespace sa {

// Installs the EndScene / Reset hooks and prepares ImGui.
// Returns false when the D3D9 device could not be located.
bool InitializeOverlay();

// Removes the hooks and shuts ImGui down.
void ShutdownOverlay();

// Toggles the menu visibility.
void ToggleMenu();
bool IsMenuOpen();
void SetMenuOpen(bool open);

// The D3D9 device the overlay draws on.
IDirect3DDevice9* GetOverlayDevice();

// Hooked game window procedure, so keyboard input can be captured.
LRESULT CALLBACK OverlayWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

}  // namespace sa
