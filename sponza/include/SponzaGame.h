#pragma once

#define _CRTDBG_MAP_ALLOC
#include <wrl/client.h>

#include <memory>
#include <utility>
#include <vector>

#include "AssetManager.h"
#include "Camera.h"
#include "DeviceResources.h"
#include "DynamicConstBuffer.h"
#include "DynamicCubeMap.h"
#include "Keyboard.h"
#include "LightHelper.h"
#include "NE_Math.h"
#include "ParticleSystem.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "ShadowMap.h"
#include "Timer.h"
#include "Actor.h"

class Game {
public:
    Game();
    ~Game();

    void Tick();
    void Initialize(HWND hWnd, uint32_t width, uint32_t height);
    void GetDefaultSize(uint32_t *width, uint32_t *height);
    void OnWindowSizeChanged(int width, int height);

private:
    void CreateDefaultSampler();
    void Clear();
    void Update();
    void Render();
    void CreateRasterizerState();
    void CreateWindowSizeDependentResources();
    void DrawMeshes(const std::vector<Mesh> &meshes);
    void DrawActors();
    void BuildShadowTransform(Mat4X4 &view, Mat4X4 &proj);

#if WITH_IMGUI
    void UpdateImgui();
#endif

    std::unique_ptr<DeviceResources> m_deviceResources;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;
    Timer m_timer;
    Camera m_camera;
    Renderer m_renderer;
    ShaderManager m_shaderManager;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    std::vector<Actor> m_actors;
    std::unique_ptr<DynamicConstBuffer> m_perFrameCB;
    std::unique_ptr<DynamicConstBuffer> m_perSceneCB;
    std::unique_ptr<DynamicConstBuffer> m_perObjectCB;
    std::unique_ptr<DynamicConstBuffer> m_perPassCB;
    std::unique_ptr<AssetManager> m_assetManager;
    ParticleSystem m_firePS;
    ShadowMap m_shadowMap;
    DynamicCubeMap m_dynamicCubeMap;
};
