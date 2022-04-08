#pragma once

#define _CRTDBG_MAP_ALLOC
#include "DeviceResources.h"
#include "Math.h"
#include "Timer.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "objloader.h"
#include "Renderer.h"
#include "ParticleSystem.h"
#include "Shader.h"
#include "D3DCommonTypes.h"

#include <vector>
#include <string>

#define MODEL_PULL 10
#define TEXTURE_PULL 4

struct PerFrameConstants
{
	Mat4X4 World;
	Mat4X4 WorldViewProj;
	Mat4X4 WorldInvTranspose;
};

struct LightingData
{
	struct PointLight PL;
	Vec3D CameraPos;
	float _Pad;
};

struct Vertex;
struct RenderData
{
	struct Vertex* Vertices;
	uint32_t NumVertices;
	uint32_t* Indices;
	size_t NumIndices;
	struct Texture DefaultTexture;
	struct Texture SpecularTexture;
	struct Texture GlossTexture;
	struct Texture NormalTexture;
	Vec3D* MeshPositions;
	uint32_t NumMeshPositions;
	ID3D11Buffer* VSConstBuffers[R_MAX_CB_NUM];
	ID3D11Buffer* PSConstBuffers[R_MAX_CB_NUM];
	struct LightingData LightingData;
	ID3D11ShaderResourceView* SRVs[TEXTURE_PULL];
};

void RenderDataInit(struct RenderData* rd, Vec3D* cameraPos);
void RenderDataDeinit(struct RenderData* rd);

struct Game
{
	Game();
	~Game();
	DeviceResources* DR;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D11PixelShader* PhongPS;
	ID3D11PixelShader* LightPS;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11InputLayout* InputLayout;
	ID3D11SamplerState* DefaultSampler;
	std::vector<Model*> Models;
	uint32_t NumMeshes;
	Timer TickTimer;
	PerFrameConstants PerFrameConstants;
	struct Camera Cam;
	struct Keyboard Keyboard;
	struct Mouse Mouse;
	Mat4X4 gWorldMat;
	Mat4X4 gViewMat;
	Mat4X4 gProjMat;
	struct RenderData RenderData;
	struct Renderer Renderer;
	ParticleSystem m_ParticleSystem;
	double m_GameTime;
};

void GameTick(Game* game);

void GameInitialize(Game* game, HWND hWnd, int width, int height);

void GameGetDefaultSize(Game* game, int* width, int* height);

void GameOnKeyDown(Game* game, WPARAM key);

void GameOnKeyUp(Game* game, WPARAM key);

void GameOnMouseMove(Game* game, uint32_t message, WPARAM wParam, LPARAM lParam);