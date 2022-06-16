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

void Game::Render()
{
#if WITH_IMGUI
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
#endif

#if WITH_IMGUI
    UpdateImgui();
#endif
    m_renderer.Clear();
#if WITH_IMGUI
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

    m_renderer.Present();
}

void Game::Tick()
{
    TimerTick(&m_timer);
    Update();
    Render();
}


void Game::Initialize(HWND hWnd, uint32_t width, uint32_t height)
{
    using namespace Microsoft::WRL;
#ifdef MATH_TEST
	MathTest();
#endif
    m_deviceResources->SetWindow(hWnd, width, height);
    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();
    TimerInitialize(&m_timer);
    Mouse::Get().SetWindowDimensions(m_deviceResources->GetBackBufferWidth(), m_deviceResources->GetBackBufferHeight());
    ID3D11Device *device = m_deviceResources->GetDevice();
    m_camera.SetViewDimensions(m_deviceResources->GetBackBufferWidth(), m_deviceResources->GetBackBufferHeight());

    UtilsCreateConstantBuffer(device, sizeof(PerSceneConstants), &m_perSceneCB);
    UtilsCreateConstantBuffer(device, sizeof(PerObjectConstants), &m_perObjectCB);
    UtilsCreateConstantBuffer(device, sizeof(PerFrameConstants), &m_perFrameCB);
    UtilsUpdateConstantBuffer(m_deviceResources->GetDeviceContext(), sizeof(PerSceneConstants), &m_perSceneData, m_perSceneCB.Get());

    m_renderer.SetDeviceResources(m_deviceResources.get());

#if WITH_IMGUI
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO();
    //const std::string droidSansTtf = UtilsFormatStr("%s/../imgui-1.87/misc/fonts/DroidSans.ttf", SRC_ROOT);
    //io.Fonts->AddFontFromFileTTF(droidSansTtf.c_str(), 16.0f);

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
