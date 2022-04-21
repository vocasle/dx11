#pragma once

#define _CRTDBG_MAP_ALLOC
#include "DeviceResources.h"
#include "Math.h"
#include "Timer.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Renderer.h"
#include "Actor.h"
#include "LightHelper.h"
#include "ShadowMap.h"

#include <vector>
#include <memory>
#include <wrl/client.h>

#define MODEL_PULL 10
#define TEXTURE_PULL 4

struct PerFrameConstants
{
	PerFrameConstants() : view{}, proj{}, cameraPosW{}, shadowTransform{}, pad{ 0 } {}
	Mat4X4 view;
	Mat4X4 proj;
	Mat4X4 shadowTransform;
	Vec3D cameraPosW;
	float pad;
};

struct PerObjectConstants
{
	PerObjectConstants() : world{}, material{} {}
	Mat4X4 world;
	Material material;
};

struct PerSceneConstants
{
	PerSceneConstants() : pointLights{}, dirLight{}, spotLights{} {}
	PointLight pointLights[4];
	DirectionalLight dirLight;
	SpotLight spotLights[2];
};

class Game
{
public:
	Game();

	~Game();

	void Tick();

	void Initialize(HWND hWnd, uint32_t width, uint32_t height);

	void GetDefaultSize(uint32_t* width, uint32_t* height);

	void OnKeyDown(WPARAM key);

	void OnKeyUp(WPARAM key);

	void OnMouseMove(uint32_t message, WPARAM wParam, LPARAM lParam);

private:
	void InitPerSceneConstants();
	void CreateDefaultSampler();
	void Clear();
	void Update();
	void Render();
	void CreateActors();
	void BuildShadowTransform();

	std::unique_ptr<DeviceResources> m_DR;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PhongPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_LightPS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
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
};