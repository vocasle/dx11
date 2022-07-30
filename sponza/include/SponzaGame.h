#pragma once

#define _CRTDBG_MAP_ALLOC
#include "Actor.h"
#include "Camera.h"
#include "DeviceResources.h"
#include "Keyboard.h"
#include "LightHelper.h"
#include "NE_Math.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "Timer.h"

#include <memory>
#include <vector>
#include <wrl/client.h>

class Game {
    public:
	Game();
	~Game();

	void Tick();
	void Initialize(HWND hWnd, uint32_t width, uint32_t height);
	void GetDefaultSize(uint32_t *width, uint32_t *height);
	void OnWindowSizeChanged(int width, int height);

    private:
	void InitPerSceneConstants();
	void CreateDefaultSampler();
	void Clear();
	void Update();
	void Render();
	void CreateActors();
	void BuildShadowTransform(Mat4X4 &view, Mat4X4 &proj);
	Actor *FindActorByName(const std::string &name);
	void DrawScene();
	void DrawSky();
	void DrawActor(const Actor &actor);
	void CreateRasterizerState();
	void CreateWindowSizeDependentResources();

#if WITH_IMGUI
	void UpdateImgui();
#endif

	std::unique_ptr<DeviceResources> m_DR;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_DefaultSampler;
	Timer m_Timer;
	Camera m_Camera;
	Renderer m_Renderer;
	std::vector<Actor> m_Actors;
	ShaderManager m_shaderManager;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};
