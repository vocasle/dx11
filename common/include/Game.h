#pragma once

#define _CRTDBG_MAP_ALLOC
#include "DeviceResources.h"
#include "NE_Math.h"
#include "Timer.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "LightHelper.h"

#include <memory>
#include <wrl/client.h>


class Game
{
public:
    virtual ~Game();
    virtual void Tick() = 0;
    virtual void Initialize(HWND hWnd, uint32_t width, uint32_t height);
    static void GetDefaultSize(uint32_t *width, uint32_t *height);
    virtual void OnWindowSizeChanged(int width, int height);

protected:
    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void CreateWindowSizeDependentResources();

#if WITH_IMGUI
    virtual void UpdateImgui() = 0;
#endif

    std::unique_ptr<DeviceResources> m_deviceResources;
    Timer m_timer;
    Camera m_camera;
    Renderer m_renderer;
    ShaderManager m_shaderManager;
};
