#pragma once
#include <iostream>
#include <WinUser.h>
#include <errhandlingapi.h>
#include <windef.h>
#include "uiaccses.h"
#include <dwmapi.h>
#include <d3d11.h>
#include "../Imgui/imgui.h"
#include "../Imgui/imgui_impl_dx11.h"
#include "../Imgui/imgui_impl_win32.h"
#include "skcrypt.hpp"
#include "../globals.h" 

// DirectX 11 objects
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

MSG messager = { NULL };
HWND my_wnd = NULL;
HWND game_wnd = NULL;

int width = GetSystemMetrics(SM_CXSCREEN);
int height = GetSystemMetrics(SM_CYSCREEN);


void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}


void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer)
    {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}


bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        &featureLevel,
        &g_pd3dDeviceContext);
    
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}



void create_overlay()
{
    // Debug logging
    std::cout << "Creating overlay window..." << std::endl;

    WNDCLASSEXA wcex = {
        sizeof(WNDCLASSEXA),
        0,
        DefWindowProcA,
        0,
        0,
        GetModuleHandle(nullptr),  // Fix: Use actual module handle
        LoadIcon(0, IDI_APPLICATION),
        LoadCursor(0, IDC_ARROW),
        0,
        0,
        "MedalOverlay",  // Fix: Use plain string instead of _("Medal")
        LoadIcon(0, IDI_APPLICATION)
    };

    // Register the window class
    ATOM rce = RegisterClassExA(&wcex);
    if (!rce)
    {
        std::cerr << "Error registering window class: " << GetLastError() << std::endl;
        return; // Fix: Actually return on failure
    }
    std::cout << "Window class registered successfully." << std::endl;

    RECT rect;
    GetWindowRect(GetDesktopWindow(), &rect);

    // Create the overlay window - Fix the creation call
    my_wnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,  // Extended styles
        "MedalOverlay",  // Class name (must match registration)
        "Medal",         // Window title
        WS_POPUP,        // Window style
        0, 0,            // Position
        rect.right, rect.bottom,  // Size
        nullptr,         // Parent window
        nullptr,         // Menu
        GetModuleHandle(nullptr),  // Instance handle
        nullptr          // Additional data
    );

    if (!my_wnd)
    {
        std::cerr << "Error creating overlay window: " << GetLastError() << std::endl;
        return;
    }
    std::cout << "Overlay window created successfully." << std::endl;

    // Initialize DirectX 11
    if (!CreateDeviceD3D(my_wnd))
    {
        std::cerr << "Error creating DirectX 11 device" << std::endl;
        CleanupDeviceD3D();
        return;
    }

    // Set window styles for overlay
    SetLayeredWindowAttributes(my_wnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    // Extend frame into client area for transparency
    MARGINS margin = { -1 };
    HRESULT hr = DwmExtendFrameIntoClientArea(my_wnd, &margin);
    if (FAILED(hr))
    {
        std::cerr << "Error extending frame into client area: " << std::hex << hr << std::endl;
        return;
    }
    std::cout << "Frame extended into client area successfully." << std::endl;

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(my_wnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ShowWindow(my_wnd, SW_SHOW);
    UpdateWindow(my_wnd);
    std::cout << "Overlay window displayed." << std::endl;
}


WPARAM render_loop() {
    ZeroMemory(&messager, sizeof(MSG));
    std::cout << ("Starting render loop...") << std::endl;
    
    while (messager.message != WM_QUIT) {
        // Process messages
        if (PeekMessage(&messager, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&messager);
            DispatchMessage(&messager);
        }
        
        // Handle window resize
        if (g_mainRenderTargetView == nullptr)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // Your ImGui rendering code goes here
        // Example:
        /* ImGui::Begin("Overlay Window");
         ImGui::Text("Hello, DirectX 11!");
         ImGui::End();*/
         RenderESP();

        // Rendering
        ImGui::Render();
        
        const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Transparent background
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        // Present
        HRESULT hr = g_pSwapChain->Present(0, 0); // Present with vsync
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // Handle device loss - recreate everything
            std::cerr << _("Device lost, recreating...") << std::endl;
            CleanupDeviceD3D();
            if (!CreateDeviceD3D(my_wnd))
            {
                std::cerr << _("Failed to recreate DirectX 11 device after loss.") << std::endl;
                break;
            }
            // Reinitialize ImGui
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
        }
    }
    
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    CleanupDeviceD3D();
    DestroyWindow(my_wnd);
    
    std::cout << _("Render loop ended.") << std::endl;
    return messager.wParam;
}
