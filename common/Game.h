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

struct PerFrameConstants
{
    PerFrameConstants()
        : Pad{0.0f}
    {
    }

    Mat4X4 View;
    Mat4X4 Proj;
    Mat4X4 ShadowTransform;
    Vec3D CameraPosW;
    float Pad;
};

struct PerObjectConstants
{
    PerObjectConstants() = default;
    Mat4X4 WorldInvTranspose;
    Mat4X4 World;
    Material Material;
};

struct PerSceneConstants
{
    PerSceneConstants() = default;
    PointLight PointLights[4];
    DirectionalLight DirLight;
    SpotLight SpotLights[2];
};

class Game
{
public:
    virtual ~Game();
    virtual void Tick() = 0;
    virtual void Initialize(HWND hWnd, uint32_t width, uint32_t height) = 0;
    static void GetDefaultSize(uint32_t *width, uint32_t *height);
    virtual void OnWindowSizeChanged(int width, int height);

protected:
    virtual void Update() = 0;
    virtual void Render();
    virtual void CreateWindowSizeDependentResources();

#if WITH_IMGUI
    virtual void UpdateImgui() = 0;
#endif

    std::unique_ptr<DeviceResources> m_deviceResources;
    Timer m_timer;
    Camera m_camera;
    Renderer m_renderer;
    PerFrameConstants m_perFrameData;
    PerObjectConstants m_perObjectData;
    PerSceneConstants m_perSceneData;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_perFrameCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_perObjectCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_perSceneCB;
    ShaderManager m_shaderManager;
};
