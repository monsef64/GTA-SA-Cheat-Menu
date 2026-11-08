#include "Hook.h"

#include <cstdio>

#include <MinHook.h>

#include "imgui.h"
#include "imgui_impl_dx9.h"

#include "Esp.h"
#include "Memory.h"
#include "Menu.h"

namespace sa {
namespace {

// The mouse position and the wheel delta are packed into the LPARAM /
// WPARAM of the window messages. The GET_X_LPARAM, GET_Y_LPARAM and
// GET_WHEEL_DELTA_WPARAM helpers normally come from <windowsx.h>, which is
// not needed here, so they are spelled out instead.
inline int GetXLParam(LPARAM lParam) {
    return static_cast<short>(static_cast<WORD>(lParam));
}

inline int GetYLParam(LPARAM lParam) {
    return static_cast<short>(static_cast<WORD>(static_cast<DWORD>(lParam) >> 16));
}

inline short GetWheelDelta(WPARAM wParam) {
    return static_cast<short>(static_cast<WORD>(wParam) >> 16);
}

// The game's main window handle is stored at 0xC17054.
constexpr uintptr_t kWindowHandleAddress = 0xC17054;

using EndSceneFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*);
using ResetFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

EndSceneFn g_originalEndScene = nullptr;
ResetFn g_originalReset = nullptr;

IDirect3DDevice9* g_device = nullptr;
HWND g_window = nullptr;
WNDPROC g_originalWndProc = nullptr;
bool g_menuOpen = true;
bool g_hooksInstalled = false;
bool g_imguiInitialized = false;

// Forward declarations of the helpers used by the hooks.
bool EnsureImGuiInitialized(IDirect3DDevice9* device);
void RenderMenu();

void DrawEspOverlay();

// Feeds ImGui the information it would normally get from a platform backend.
//
// There is no ImGui Win32 backend here: the overlay is drawn straight onto
// the game's own D3D9 device, so the values a platform backend would set up
// have to be filled in by hand. io.DisplaySize in particular is required -
// without it ImGui builds its draw data with a zero sized viewport, which
// trips the "Invalid DisplaySize value!" assertion in imgui.cpp.
void UpdateImGuiFrameState(IDirect3DDevice9* device) {
    if (device == nullptr) return;

    ImGuiIO& io = ImGui::GetIO();

    // The draw data is sized from io.DisplaySize, and the DX9 backend turns
    // that into the back buffer viewport it renders into.
    D3DVIEWPORT9 viewport;
    if (SUCCEEDED(device->GetViewport(&viewport)) && viewport.Width > 0 &&
        viewport.Height > 0) {
        io.DisplaySize = ImVec2(static_cast<float>(viewport.Width),
                                static_cast<float>(viewport.Height));
    }

    // The game renders at native resolution, so there is no scaling.
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // ImGui uses the delta time for animations and for things like the
    // double-click interval. The game runs at a fixed frame rate, but the
    // actual frame time is not known here, so a sensible default is used.
    // A zero delta would make every animation-based widget freeze.
    if (io.DeltaTime <= 0.0f) io.DeltaTime = 1.0f / 60.0f;
}

// --- Hooked functions -------------------------------------------------------

HRESULT STDMETHODCALLTYPE HookedEndScene(IDirect3DDevice9* device) {
    if (g_device == nullptr) g_device = device;

    if (g_originalEndScene != nullptr) {
        const HRESULT hr = g_originalEndScene(device);
        if (FAILED(hr)) return hr;
    }

    if (!EnsureImGuiInitialized(device)) return S_OK;

    // The display size has to be known before the frame is started,
    // otherwise ImGui builds its draw data with a zero sized viewport.
    UpdateImGuiFrameState(device);

    ImGui_ImplDX9_NewFrame();
    ImGui::NewFrame();

    // The ESP is drawn inside the ImGui frame so it can use the draw list for
    // the text labels. The menu is drawn afterwards so it sits on top.
    DrawEspOverlay();

    if (g_menuOpen) {
        RenderMenu();
    }

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HookedReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {
    if (g_imguiInitialized) ImGui_ImplDX9_InvalidateDeviceObjects();
    const HRESULT hr = g_originalReset != nullptr ? g_originalReset(device, params) : S_OK;
    if (g_imguiInitialized) ImGui_ImplDX9_CreateDeviceObjects();
    return hr;
}

// --- Input forwarding -------------------------------------------------------

// Maps a Win32 virtual key code onto the matching ImGui key.
//
// ImGui 1.90 does not ship a Win32 key translation table, so the mapping is
// written out here. Only the keys the menu actually needs are covered:
// letters, digits, the editing keys, the modifiers and the function keys.
ImGuiKey TranslateVirtualKey(WPARAM virtualKey) {
    switch (virtualKey) {
        case VK_BACK:   return ImGuiKey_Backspace;
        case VK_TAB:    return ImGuiKey_Tab;
        case VK_RETURN: return ImGuiKey_Enter;
        case VK_PAUSE:  return ImGuiKey_Pause;
        case VK_CAPITAL: return ImGuiKey_CapsLock;
        case VK_ESCAPE: return ImGuiKey_Escape;
        case VK_SPACE:  return ImGuiKey_Space;
        case VK_PRIOR:  return ImGuiKey_PageUp;
        case VK_NEXT:   return ImGuiKey_PageDown;
        case VK_END:    return ImGuiKey_End;
        case VK_HOME:   return ImGuiKey_Home;
        case VK_LEFT:   return ImGuiKey_LeftArrow;
        case VK_RIGHT:  return ImGuiKey_RightArrow;
        case VK_UP:     return ImGuiKey_UpArrow;
        case VK_DOWN:   return ImGuiKey_DownArrow;
        case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
        case VK_INSERT: return ImGuiKey_Insert;
        case VK_DELETE: return ImGuiKey_Delete;
        case VK_OEM_1:  return ImGuiKey_Semicolon;
        case VK_OEM_PLUS: return ImGuiKey_Equal;
        case VK_OEM_COMMA: return ImGuiKey_Comma;
        case VK_OEM_MINUS: return ImGuiKey_Minus;
        case VK_OEM_PERIOD: return ImGuiKey_Period;
        case VK_OEM_2:  return ImGuiKey_Slash;
        case VK_OEM_3:  return ImGuiKey_GraveAccent;
        case VK_OEM_4:  return ImGuiKey_LeftBracket;
        case VK_OEM_5:  return ImGuiKey_Backslash;
        case VK_OEM_6:  return ImGuiKey_RightBracket;
        case VK_OEM_7:  return ImGuiKey_Apostrophe;
        case VK_NUMPAD0: return ImGuiKey_Keypad0;
        case VK_NUMPAD1: return ImGuiKey_Keypad1;
        case VK_NUMPAD2: return ImGuiKey_Keypad2;
        case VK_NUMPAD3: return ImGuiKey_Keypad3;
        case VK_NUMPAD4: return ImGuiKey_Keypad4;
        case VK_NUMPAD5: return ImGuiKey_Keypad5;
        case VK_NUMPAD6: return ImGuiKey_Keypad6;
        case VK_NUMPAD7: return ImGuiKey_Keypad7;
        case VK_NUMPAD8: return ImGuiKey_Keypad8;
        case VK_NUMPAD9: return ImGuiKey_Keypad9;
        case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case VK_ADD:      return ImGuiKey_KeypadAdd;
        case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case VK_DECIMAL:  return ImGuiKey_KeypadDecimal;
        case VK_DIVIDE:   return ImGuiKey_KeypadDivide;
        case VK_NUMLOCK:  return ImGuiKey_NumLock;
        case VK_SCROLL:   return ImGuiKey_ScrollLock;
        case VK_LCONTROL: return ImGuiKey_LeftCtrl;
        case VK_LSHIFT:   return ImGuiKey_LeftShift;
        case VK_LMENU:    return ImGuiKey_LeftAlt;
        case VK_RCONTROL: return ImGuiKey_RightCtrl;
        case VK_RSHIFT:   return ImGuiKey_RightShift;
        case VK_RMENU:    return ImGuiKey_RightAlt;
        case VK_APPS:     return ImGuiKey_Menu;
        default: break;
    }

    // '0' - '9'
    if (virtualKey >= '0' && virtualKey <= '9') {
        return static_cast<ImGuiKey>(static_cast<int>(ImGuiKey_0) +
                                     (static_cast<int>(virtualKey) - '0'));
    }
    // 'A' - 'Z'
    if (virtualKey >= 'A' && virtualKey <= 'Z') {
        return static_cast<ImGuiKey>(static_cast<int>(ImGuiKey_A) +
                                     (static_cast<int>(virtualKey) - 'A'));
    }
    // F1 - F12
    if (virtualKey >= VK_F1 && virtualKey <= VK_F12) {
        return static_cast<ImGuiKey>(static_cast<int>(ImGuiKey_F1) +
                                     (static_cast<int>(virtualKey) - VK_F1));
    }

    return ImGuiKey_None;
}

// Translates a Win32 mouse message into the matching ImGui event.
// Returns true when the message was consumed by the overlay.
bool ForwardMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    ImGuiIO& io = ImGui::GetIO();

    switch (msg) {
        case WM_MOUSEMOVE:
            io.AddMousePosEvent(static_cast<float>(GetXLParam(lParam)),
                               static_cast<float>(GetYLParam(lParam)));
            return io.WantCaptureMouse;
        case WM_LBUTTONDOWN:
            io.AddMouseButtonEvent(0, true);
            return io.WantCaptureMouse;
        case WM_LBUTTONUP:
            io.AddMouseButtonEvent(0, false);
            return io.WantCaptureMouse;
        case WM_RBUTTONDOWN:
            io.AddMouseButtonEvent(1, true);
            return io.WantCaptureMouse;
        case WM_RBUTTONUP:
            io.AddMouseButtonEvent(1, false);
            return io.WantCaptureMouse;
        case WM_MBUTTONDOWN:
            io.AddMouseButtonEvent(2, true);
            return io.WantCaptureMouse;
        case WM_MBUTTONUP:
            io.AddMouseButtonEvent(2, false);
            return io.WantCaptureMouse;
        case WM_MOUSEWHEEL: {
            const float wheel = static_cast<float>(GetWheelDelta(wParam));
            io.AddMouseWheelEvent(0.0f, wheel);
            return io.WantCaptureMouse;
        }
        default:
            return false;
    }
}

// Translates a Win32 keyboard message into the matching ImGui event.
// Returns true when the message was consumed by the overlay.
bool ForwardKeyboardMessage(UINT msg, WPARAM wParam) {
    ImGuiIO& io = ImGui::GetIO();

    if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
        // The Insert key is reserved for toggling the menu.
        if (wParam != VK_INSERT) {
            const ImGuiKey key = TranslateVirtualKey(wParam);
            if (key != ImGuiKey_None) io.AddKeyEvent(key, true);
        }
        return io.WantCaptureKeyboard;
    }

    if (msg == WM_KEYUP || msg == WM_SYSKEYUP) {
        if (wParam != VK_INSERT) {
            const ImGuiKey key = TranslateVirtualKey(wParam);
            if (key != ImGuiKey_None) io.AddKeyEvent(key, false);
        }
        return io.WantCaptureKeyboard;
    }

    if (msg == WM_CHAR) {
        // WM_CHAR already carries the translated character, so it is forwarded
        // as-is. That keeps the text fields working for non-ASCII layouts.
        const unsigned int ch = static_cast<unsigned int>(wParam);
        if (ch >= 32 && ch != 127) io.AddInputCharacter(ch);
        return io.WantTextInput;
    }

    return false;
}

// --- Window procedure --------------------------------------------------------

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Toggle the menu with the Insert key. This is handled before anything
    // else so that the menu can always be closed again.
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        ToggleMenu();
        return 0;
    }

    if (g_imguiInitialized && ImGui::GetCurrentContext() != nullptr) {
        // The overlay only reacts to input while it is open.
        if (g_menuOpen) {
            if (ForwardMouseMessage(msg, wParam, lParam)) return 0;
            if (ForwardKeyboardMessage(msg, wParam)) return 0;
        }
    }

    return CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
}

// Creates a temporary device so we can read the vtable and hook it.
IDirect3DDevice9* CreateDummyDevice() {
    HMODULE d3d9 = LoadLibraryA("d3d9.dll");
    if (d3d9 == nullptr) return nullptr;

    using Direct3DCreate9Fn = IDirect3D9*(STDMETHODCALLTYPE*)(UINT);
    auto createFn = reinterpret_cast<Direct3DCreate9Fn>(GetProcAddress(d3d9, "Direct3DCreate9"));
    if (createFn == nullptr) {
        FreeLibrary(d3d9);
        return nullptr;
    }

    IDirect3D9* d3d9Interface = createFn(D3D_SDK_VERSION);
    if (d3d9Interface == nullptr) {
        FreeLibrary(d3d9);
        return nullptr;
    }

    D3DPRESENT_PARAMETERS params;
    ZeroMemory(&params, sizeof(params));
    params.Windowed = TRUE;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow = GetDesktopWindow();
    params.BackBufferFormat = D3DFMT_UNKNOWN;

    IDirect3DDevice9* device = nullptr;
    const HRESULT hr = d3d9Interface->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, nullptr,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &device);
    d3d9Interface->Release();
    FreeLibrary(d3d9);

    if (FAILED(hr)) return nullptr;
    return device;
}

bool InstallHooks() {
    IDirect3DDevice9* dummy = CreateDummyDevice();
    if (dummy == nullptr) return false;

    // EndScene is vtable slot 42, Reset is slot 16.
    void** vtable = *reinterpret_cast<void***>(dummy);
    void* endScenePtr = vtable[42];
    void* resetPtr = vtable[16];

    dummy->Release();

    if (MH_Initialize() != MH_OK) return false;

    if (MH_CreateHook(endScenePtr, reinterpret_cast<void*>(&HookedEndScene),
                      reinterpret_cast<void**>(&g_originalEndScene)) != MH_OK) {
        MH_Uninitialize();
        return false;
    }
    if (MH_CreateHook(resetPtr, reinterpret_cast<void*>(&HookedReset),
                      reinterpret_cast<void**>(&g_originalReset)) != MH_OK) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        return false;
    }

    MH_EnableHook(MH_ALL_HOOKS);
    return true;
}

// The game stores its main window handle at 0xC17054.
HWND FindGameWindow() {
    const uintptr_t handle = ReadPointer(kWindowHandleAddress);
    if (handle == 0) return nullptr;
    return reinterpret_cast<HWND>(handle);
}

bool EnsureImGuiInitialized(IDirect3DDevice9* device) {
    if (g_imguiInitialized) return true;

    g_window = FindGameWindow();
    if (g_window == nullptr) g_window = GetForegroundWindow();

    if (g_window != nullptr) {
        g_originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
            g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HookedWndProc)));
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;  // do not write an ini file
    ImGui::StyleColorsDark();

    if (!ImGui_ImplDX9_Init(device)) {
        ImGui::DestroyContext();
        return false;
    }

    g_imguiInitialized = true;
    return true;
}

void RenderMenu() {
    DrawMainMenu();
}

void DrawEspOverlay() {
    if (!IsEspEnabled()) return;
    UpdateEsp();
    DrawEsp();
}

}  // namespace

bool InitializeOverlay() {
    if (g_hooksInstalled) return true;

    if (!InstallHooks()) {
        std::printf("[sa-menu] failed to install the D3D9 hooks\n");
        return false;
    }

    g_hooksInstalled = true;
    return true;
}

void ShutdownOverlay() {
    if (!g_hooksInstalled) return;

    if (g_imguiInitialized) {
        ImGui_ImplDX9_Shutdown();
        g_imguiInitialized = false;
    }
    if (ImGui::GetCurrentContext() != nullptr) ImGui::DestroyContext();

    if (g_originalWndProc != nullptr && g_window != nullptr) {
        SetWindowLongPtrA(g_window, GWLP_WNDPROC,
                          reinterpret_cast<LONG_PTR>(g_originalWndProc));
        g_originalWndProc = nullptr;
    }

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    g_hooksInstalled = false;
}

void ToggleMenu() { g_menuOpen = !g_menuOpen; }
bool IsMenuOpen() { return g_menuOpen; }
void SetMenuOpen(bool open) { g_menuOpen = open; }

IDirect3DDevice9* GetOverlayDevice() { return g_device; }

LRESULT CALLBACK OverlayWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_originalWndProc != nullptr) {
        return CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
    }
    return 0;
}

}  // namespace sa
