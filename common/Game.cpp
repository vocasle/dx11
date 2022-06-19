#include "Game.h"
#include "Utils.h"
#include "NE_Math.h"
#include "Camera.h"
#include "Mouse.h"

#include <d3dcompiler.h>

#if WITH_IMGUI
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#endif

#include <sstream>

using namespace Microsoft::WRL;


Game::~Game()
{
#if WITH_IMGUI
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif
}

void Game::Initialize(HWND hWnd, uint32_t width, uint32_t height)
{
#ifdef MATH_TEST
    MathTest();
#endif
    m_deviceResources->SetWindow(hWnd, width, height);
    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();
    TimerInitialize(&m_timer);
    Mouse::Get().SetWindowDimensions(m_deviceResources->GetBackBufferWidth(), m_deviceResources->GetBackBufferHeight());
    m_camera.SetViewDimensions(m_deviceResources->GetBackBufferWidth(), m_deviceResources->GetBackBufferHeight());
    m_renderer.SetDeviceResources(m_deviceResources.get());

#if WITH_IMGUI
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(m_deviceResources->GetDevice(), m_deviceResources->GetDeviceContext());
#endif
}

void Game::GetDefaultSize(uint32_t *width, uint32_t *height)
{
    *width = DEFAULT_WIN_WIDTH;
    *height = DEFAULT_WIN_HEIGHT;
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::CreateWindowSizeDependentResources()
{
    auto const size = m_deviceResources->GetOutputSize();
    const float aspectRatio = static_cast<float>(size.right) / static_cast<float>(size.bottom);
    float fovAngleY = 45.0f;

    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    m_camera.SetFov(fovAngleY);
    m_camera.SetViewDimensions(size.right, size.bottom);
}
