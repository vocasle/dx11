#pragma once

#define _CRTDBG_MAP_ALLOC
#include "Actor.h"
#include "Camera.h"
#include "CubeMap.h"
#include "DeviceResources.h"
#include "DynamicCubeMap.h"
#include "Keyboard.h"
#include "LightHelper.h"
#include "NE_Math.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "ShadowMap.h"
#include "Timer.h"

#include <memory>
#include <vector>
#include <wrl/client.h>

#define MODEL_PULL 10
#define TEXTURE_PULL 4

struct PerFrameConstants {
	PerFrameConstants()
		: pad{ 0.0f }
	{
	}
	Mat4X4 view;
	Mat4X4 proj;
	Mat4X4 shadowTransform;
	Vec3D cameraPosW;
	float pad;
};

struct PerObjectConstants {
	PerObjectConstants()
	{
	}
	Mat4X4 worldInvTranspose;
	Mat4X4 world;
	Material material;
};

struct PerSceneConstants {
	PerSceneConstants()
		: pointLights{}
		, spotLights{}
	{
	}
	alignas(16) PointLight pointLights[4];
    alignas(16) DirectionalLight dirLight;
    alignas(16) SpotLight spotLights[2];
    alignas(16) Color fogColor;
    alignas(4) float fogStart;
    alignas(4) float zFar;
    alignas(4) float fogDensity;
};

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

	// new stuff
	std::vector<Actor> m_Actors;
	PerFrameConstants m_PerFrameData;
	PerObjectConstants m_PerObjectData;
	PerSceneConstants m_PerSceneData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_PerFrameCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_PerObjectCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_PerSceneCB;
	ShadowMap m_ShadowMap;
	CubeMap m_CubeMap;
	DynamicCubeMap m_dynamicCubeMap;
	ShaderManager m_shaderManager;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};
