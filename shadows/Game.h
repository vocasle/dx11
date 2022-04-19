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

#define MODEL_PULL 10
#define TEXTURE_PULL 4

typedef struct PerFrameConstants
{
	Mat4X4 view;
	Mat4X4 proj;
	Vec3D cameraPosW;
	float pad;
} PerFrameConstants;

typedef struct PerObjectConstants
{
	Mat4X4 world;
	Material material;
} PerObjectConstants;

typedef struct PerSceneConstants
{
	PointLight pointLights[4];
	DirectionalLight dirLight;
	SpotLight spotLights[2];
} PerSceneConstants;

struct LightingData
{
	PointLight PL[4];
	Vec3D CameraPos;
	float _Pad;
};

typedef struct Game
{
	DeviceResources* DR;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D11PixelShader* PhongPS;
	ID3D11PixelShader* LightPS;
	ID3D11InputLayout* InputLayout;
	ID3D11SamplerState* DefaultSampler;
	Timer TickTimer;
	PerFrameConstants PerFrameConstants;
	struct Camera Cam;
	struct Keyboard Keyboard;
	struct Mouse Mouse;
	Mat4X4 gWorldMat;
	Mat4X4 gViewMat;
	Mat4X4 gProjMat;
	struct Renderer Renderer;

	// new stuff
	Actor** m_Actors;
	size_t m_NumActors;
	PerFrameConstants m_PerFrameData;
	PerObjectConstants m_PerObjectData;
	PerSceneConstants m_PerSceneData;
	ID3D11Buffer* m_PerFrameCB;
	ID3D11Buffer* m_PerObjectCB;
	ID3D11Buffer* m_PerSceneCB;
} Game;

Game* GameNew(void);

void GameFree(Game* game);

void GameTick(Game* game);

void GameInitialize(Game* game, HWND hWnd, int width, int height);

void GameGetDefaultSize(Game* game, int* width, int* height);

void GameOnKeyDown(Game* game, WPARAM key);

void GameOnKeyUp(Game* game, WPARAM key);

void GameOnMouseMove(Game* game, uint32_t message, WPARAM wParam, LPARAM lParam);