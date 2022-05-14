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
#include "CubeMap.h"
#include "InputLayout.h"

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
	PerObjectConstants() : worldInvTranspose{}, world {}, material{} {}
	Mat4X4 worldInvTranspose;
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

private:
	void InitPerSceneConstants();
	void CreateDefaultSampler();
	void Clear();
	void Update();
	void Render();
	void CreateActors();
	void BuildShadowTransform();
	std::vector<uint8_t> CreateVertexShader(const char* filepath, ID3D11Device* device, ID3D11VertexShader** vs);
	void CreatePixelShader(const char* filepath, ID3D11Device* device, ID3D11PixelShader** ps);
	const Actor* FindActorByName(const std::string& name) const;

#if WITH_IMGUI
	void UpdateImgui(); 

	struct ImguiState
	{
		ImguiState() : RotateDirLight{ false }, AnimatePointLight{ false }, ToggleSpotlight{ false }{}
		bool RotateDirLight;
		bool AnimatePointLight;
		bool ToggleSpotlight;
	};

	ImguiState m_ImguiState;
#endif

	struct BoundingSphere
	{
		BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
		Vec3D Center;
		float Radius;
	};

	BoundingSphere m_sceneBounds;

	std::unique_ptr<DeviceResources> m_DR;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PhongPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_LightPS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_SkyVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_SkyPS;
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
	InputLayout m_InputLayout;
};