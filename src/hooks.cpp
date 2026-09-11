#include <windows.h>
#include <d3d9.h>
#include <MinHook.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include "menu.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

typedef HRESULT(APIENTRY* EndScene_t)(LPDIRECT3DDEVICE9);
static EndScene_t oEndScene = nullptr;

typedef LRESULT(CALLBACK* WndProc_t)(HWND, UINT, WPARAM, LPARAM);
static WndProc_t oWndProc = nullptr;

static bool g_imguiInitialized = false;
static HWND g_gameWindow = nullptr;

static LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

static HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
    if (!g_imguiInitialized) {
        D3DDEVICE_CREATION_PARAMETERS params;
        pDevice->GetCreationParameters(&params);
        g_gameWindow = params.hFocusWindow;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr; // не сохранять layout окон на диск

        ImGui_ImplWin32_Init(g_gameWindow);
        ImGui_ImplDX9_Init(pDevice);

        oWndProc = (WndProc_t)SetWindowLongPtr(g_gameWindow, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

        g_imguiInitialized = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RenderMenu();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

// Создаём временное D3D9-устройство только чтобы достать адрес vtable
static uintptr_t* GetD3D9DeviceVTable() {
    IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) return nullptr;

    D3DPRESENT_PARAMETERS pp = {};
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = GetForegroundWindow();

    IDirect3DDevice9* device = nullptr;
    HRESULT hr = d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);

    if (FAILED(hr) || !device) {
        d3d->Release();
        return nullptr;
    }

    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(device);

    device->Release();
    d3d->Release();

    return vtable;
}

void InitHooks() {
    uintptr_t* vtable = GetD3D9DeviceVTable();
    if (!vtable) return;

    MH_Initialize();
    MH_CreateHook(reinterpret_cast<LPVOID>(vtable[42]), // EndScene
                  reinterpret_cast<LPVOID>(&hkEndScene),
                  reinterpret_cast<LPVOID*>(&oEndScene));
    MH_EnableHook(reinterpret_cast<LPVOID>(vtable[42]));
}
