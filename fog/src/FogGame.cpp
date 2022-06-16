#include "FogGame.h"

FogGame::FogGame()
{
    m_deviceResources = std::make_unique<DeviceResources>();
}

FogGame::~FogGame()
{
}

void FogGame::Tick()
{
    Game::Tick();
}

void FogGame::Initialize(HWND hWnd, uint32_t width, uint32_t height)
{
    Game::Initialize(hWnd, width, height);
}

void FogGame::OnWindowSizeChanged(int width, int height)
{
    Game::OnWindowSizeChanged(width, height);
}

void FogGame::Update()
{
     
}

void FogGame::Render()
{
    Game::Render();
}

void FogGame::CreateWindowSizeDependentResources()
{
}

void FogGame::UpdateImgui()
{
}
