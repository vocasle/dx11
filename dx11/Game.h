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

#define MODEL_PULL 10
#define TEXTURE_PULL 4

typedef struct PerFrameConstants
{
	Mat4X4 World;
	Mat4X4 View;
	Mat4X4 Proj;
} PerFrameConstants;

struct PointLight
{
	Vec4D Ambient;
	Vec4D Diffuse;
	Vec4D Specular;
	Vec3D Position;
	float _Pad;
};

struct Material
{
	Vec4D Ambient;
	Vec4D Diffuse;
	Vec4D Specular;
};

struct LightingData
{
	struct PointLight PL;
	Vec3D CameraPos;
	float _Pad;
};

struct Texture
{
	ID3D11Texture2D* Resource;
	ID3D11ShaderResourceView* SRV;
};

void TextureDeinit(struct Texture* texture);

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

typedef struct Game
{
	DeviceResources* DR;
	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D11PixelShader* PhongPS;
	ID3D11PixelShader* LightPS;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11InputLayout* InputLayout;
	//ID3D11Buffer* PerFrameConstantsCB;
	ID3D11SamplerState* DefaultSampler;
	struct Model** Models;
	uint32_t NumModels;
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
} Game;

Game* GameNew(void);

void GameFree(Game* game);

void GameTick(Game* game);

void GameInitialize(Game* game, HWND hWnd, int width, int height);

void GameGetDefaultSize(Game* game, int* width, int* height);

void GameOnKeyDown(Game* game, WPARAM key);

void GameOnKeyUp(Game* game, WPARAM key);

void GameOnMouseMove(Game* game, uint32_t message, WPARAM wParam, LPARAM lParam);

void GameLoadTextureFromFile(DeviceResources* dr, const char* filename, struct Texture* texture);
