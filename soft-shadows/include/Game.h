#pragma once

#define _CRTDBG_MAP_ALLOC
#include "DeviceResources.h"
#include "NE_Math.h"
#include "Timer.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Renderer.h"
#include "Actor.h"
#include "LightHelper.h"
#include "ShadowMap.h"
#include "CubeMap.h"
#include "DynamicCubeMap.h"
#include "ParticleSystem.h"
#include "ShaderManager.h"

#include <vector>
#include <memory>
#include <wrl/client.h>

#define MODEL_PULL 10
#define TEXTURE_PULL 4

struct PerFrameConstants
{
	PerFrameConstants() : pad{ 0.0f } {}
	Mat4X4 view;
	Mat4X4 proj;
	Mat4X4 shadowTransform;
	Vec3D cameraPosW;
	float pad;
};

struct PerObjectConstants
{
	PerObjectConstants() {}
	Mat4X4 worldInvTranspose;
	Mat4X4 world;
	Material material;
};

struct PerSceneConstants
{
	PerSceneConstants() : pointLights{}, spotLights{} {}
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
	Actor* FindActorByName(const std::string& name);
	void DrawScene();
	void DrawSky();
	void DrawActor(const Actor& actor);

#if WITH_IMGUI
	void UpdateImgui();
#endif

	struct BoundingSphere
	{
		BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
		Vec3D Center;
		float Radius;
	};

	BoundingSphere m_sceneBounds;

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
	ParticleSystem m_particleSystem;
	ShaderManager m_shaderManager;
};
