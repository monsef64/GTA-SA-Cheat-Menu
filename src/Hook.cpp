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

// --- Hooked functions -------------------------------------------------------

HRESULT STDMETHODCALLTYPE HookedEndScene(IDirect3DDevice9* device) {
    if (g_device == nullptr) g_device = device;

    if (g_originalEndScene != nullptr) {
        const HRESULT hr = g_originalEndScene(device);
        if (FAILED(hr)) return hr;
    }

    if (!EnsureImGuiInitialized(device)) return S_OK;

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

// --- Window procedure --------------------------------------------------------

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_imguiInitialized && ImGui::GetCurrentContext() != nullptr) {
        // Let ImGui swallow the input while the menu is open.
        if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) {
            switch (msg) {
                case WM_MOUSEMOVE:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MOUSEWHEEL:
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_CHAR:
                    return 0;
                default:
                    break;
            }
        }
    }

    // Toggle the menu with the Insert key.
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        ToggleMenu();
        return 0;
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
