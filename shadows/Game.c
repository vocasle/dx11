#include "Game.h"
#include "Utils.h"
#include "Math.h"
#include "Camera.h"
#include "stb_image.h"

#include <windowsx.h>

struct Vertex
{
	Vec3D Position;
	Vec3D Normal;
	Vec2D TexCoords;
};

void GameCreateDefaultSampler(Game* game)
{
	D3D11_SAMPLER_DESC samplerDesc = { 0 };
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11Device1* device = game->DR->Device;

	if (FAILED(device->lpVtbl->CreateSamplerState(device, &samplerDesc, &game->DefaultSampler)))
	{
		UtilsDebugPrint("ERROR: Failed to create default sampler state\n");
		ExitProcess(EXIT_FAILURE);
	}
}

void TextureDeinit(struct Texture* texture)
{
	COM_FREE(texture->Resource);
	COM_FREE(texture->SRV);
}

void RenderDataInit(struct RenderData* rd, Vec3D* cameraPos)
{
	memset(rd, 0, sizeof(struct RenderData));
	struct PointLight pl = { 0 };
	pl.Position = MathVec3DFromXYZ(2.0f, 0.0f, -2.0f);
	pl.Ambient = ColorFromRGBA(0.1f, 0.1f, 0.1f, 1.0f);
	pl.Diffuse = ColorFromRGBA(0.5f, 0.5f, 0.5f, 1.0f);
	pl.Specular = ColorFromRGBA(1.0f, 1.0f, 1.0f, 32.0f);
	rd->LightingData.PL = pl;
	rd->LightingData.CameraPos = *cameraPos;
}

void RenderDataDeinit(struct RenderData* rd)
{
	TextureDeinit(&rd->DefaultTexture);
	TextureDeinit(&rd->SpecularTexture);
	TextureDeinit(&rd->GlossTexture);
	TextureDeinit(&rd->NormalTexture);
	free(rd->Vertices);
	free(rd->Indices);
	free(rd->MeshPositions);
	rd->NumMeshPositions = 0;
	rd->NumVertices = 0;
	rd->NumIndices = 0;
	for (uint32_t i = 0; i < R_MAX_CB_NUM; ++i)
	{
		if (rd->PSConstBuffers[i])
		{
			COM_FREE(rd->PSConstBuffers[i]);
		}

		if (rd->VSConstBuffers[i])
		{
			COM_FREE(rd->VSConstBuffers[i]);
		}
	}
}

Game* GameNew(void)
{
	Game* g = malloc(sizeof(Game));
	memset(g, 0, sizeof(Game));
	g->DR = DRNew();
	const size_t bytes = sizeof(struct Model) * MODEL_PULL;
	g->Models = malloc(bytes);
	memset(g->Models, 0, bytes);

	return g;
}

void GameFree(Game* game)
{
	COM_FREE(game->VS);
	COM_FREE(game->PS);
	COM_FREE(game->InputLayout);
	COM_FREE(game->DefaultSampler);
	COM_FREE(game->PhongPS);
	COM_FREE(game->LightPS);
	RenderDataDeinit(&game->RenderData);
	KeyboardDeinit(&game->Keyboard);
	DRFree(game->DR);
	for (uint32_t i = 0; i < game->NumModels; ++i)
	{
		ModelFree(game->Models[i]);
	}
	free(game->Models);
	for (size_t i = 0; i < game->m_NumActors; ++i)
	{
		ActorFree(game->m_Actors[i]);
	}
	free(game->m_Actors);
	
	memset(game, 0, sizeof(Game));
	free(game);
	DRReportLiveObjects();
}

static void GameClear(Game* game)
{
	ID3D11DeviceContext1* ctx = game->DR->Context;
	ID3D11RenderTargetView* rtv = game->DR->RenderTargetView;
	ID3D11DepthStencilView* dsv = game->DR->DepthStencilView;

	static const float CLEAR_COLOR[4] = {0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f};

	ctx->lpVtbl->Flush(ctx);

	ctx->lpVtbl->ClearRenderTargetView(ctx, rtv, CLEAR_COLOR);
	ctx->lpVtbl->ClearDepthStencilView(ctx, dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ctx->lpVtbl->OMSetRenderTargets(ctx, 1, &rtv, dsv);
	ctx->lpVtbl->RSSetViewports(ctx, 1, &game->DR->ScreenViewport);
}

//static void GameRender(Game* game)
//{
//	ID3D11DeviceContext1* ctx = game->DR->Context;
//
//	GameClear(game);
//
//	const UINT strides = sizeof(struct Vertex);
//	const UINT offsets = 0;
//
//	ctx->lpVtbl->IASetPrimitiveTopology(ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	ctx->lpVtbl->IASetInputLayout(ctx, game->InputLayout);
//	ctx->lpVtbl->IASetVertexBuffers(ctx, 0, 1, &game->VertexBuffer, &strides, &offsets);
//	ctx->lpVtbl->IASetIndexBuffer(ctx, game->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//	ctx->lpVtbl->RSSetState(ctx, (ID3D11RasterizerState*) game->DR->RasterizerState);
//	ctx->lpVtbl->PSSetSamplers(ctx, 0, 1, &game->DefaultSampler);
//	ctx->lpVtbl->VSSetShader(ctx, game->VS, NULL, 0);
//	ctx->lpVtbl->PSSetShader(ctx, game->DefaultPS, NULL, 0);
//
//	uint32_t vertexOffset = 0;
//	uint32_t indexOffset = 0;
//	uint32_t meshGlobIdx = 0;
//	ID3D11ShaderResourceView* nullSRV[] = {NULL};
//	for (uint32_t j = 0; j < game->NumModels; ++j)
//	{
//		struct Model* model = game->Models[j];
//		for (uint32_t i = 0; i < model->NumMeshes; ++i)
//		{
//			const struct Mesh* mesh = model->Meshes + i;
//			const Vec3D offset = game->RenderData.MeshPositions[meshGlobIdx];
//			game->PerFrameConstants.World = MathMat4X4TranslateFromVec3D(&offset);
//
//			{
//				D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
//
//				ID3D11DeviceContext1* ctx = game->DR->Context;
//				if (FAILED(ctx->lpVtbl->Map(ctx, (ID3D11Resource*)game->PerFrameConstantsCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
//				{
//					OutputDebugStringA("ERROR: Failed to map per frame constants cbuffer\n");
//					ExitProcess(EXIT_FAILURE);
//				}
//				memcpy(mapped.pData, &game->PerFrameConstants, sizeof(PerFrameConstants));
//				ctx->lpVtbl->Unmap(ctx, (ID3D11Resource*)game->PerFrameConstantsCB, 0);
//			}
//
//			if (game->RenderData.DefaultTexture.SRV)
//			{
//				ctx->lpVtbl->PSSetShaderResources(ctx, 0, 1, &game->RenderData.DefaultTexture.SRV);
//				if (game->RenderData.SpecularTexture.SRV)
//				{
//					ctx->lpVtbl->PSSetShaderResources(ctx, 1, 1, &game->RenderData.SpecularTexture.SRV);
//				}
//				if (game->RenderData.GlossTexture.SRV)
//				{
//					ctx->lpVtbl->PSSetShaderResources(ctx, 2, 1, &game->RenderData.GlossTexture.SRV);
//				}
//				if (game->RenderData.NormalTexture.SRV)
//				{
//					ctx->lpVtbl->PSSetShaderResources(ctx, 3, 1, &game->RenderData.NormalTexture.SRV);
//				}
//			}
//			else
//			{
//				ctx->lpVtbl->PSSetShaderResources(ctx, 0, 1, nullSRV);
//			}
//			ctx->lpVtbl->VSSetConstantBuffers(ctx, 0, 1, &game->PerFrameConstantsCB);
//			ctx->lpVtbl->DrawIndexed(ctx, (uint32_t)mesh->NumFaces, indexOffset, (int32_t)vertexOffset);
//			indexOffset += mesh->NumFaces;
//			vertexOffset += mesh->NumFaces;
//			meshGlobIdx++;
//		}
//	}
//
//	const HRESULT hr = game->DR->SwapChain->lpVtbl->Present(game->DR->SwapChain, 1, 0);
//
//	ctx->lpVtbl->DiscardView(ctx, (ID3D11View*) game->DR->RenderTargetView);
//	if (game->DR->DepthStencilView)
//	{
//		ctx->lpVtbl->DiscardView(ctx, (ID3D11View*)game->DR->DepthStencilView);
//	}
//
//	if (FAILED(hr))
//	{
//		OutputDebugStringA("ERROR: Failed to present\n");
//		ExitProcess(EXIT_FAILURE);
//	}
//}
static void GameUpdateConstantBuffer(ID3D11DeviceContext* context,
	size_t bufferSize,
	void* data,
	ID3D11Buffer* dest);
static void GameUpdatePerFrameConstants(Game* game);
void GameUpdate(Game* game)
{
	CameraUpdatePos(&game->Cam, game->TickTimer.DeltaMillis);
	CameraProcessMouse(&game->Cam, game->TickTimer.DeltaMillis);

	const float width = (float)game->DR->BackbufferWidth;
	const float height = (float)game->DR->BackbufferHeight;

	game->gViewMat = CameraGetViewMat(&game->Cam);
	game->gProjMat = MathMat4X4PerspectiveFov(MathToRadians(45.0f), width / height, 0.1f, 100.0f);
	
	game->PerFrameConstants.World = game->gWorldMat;
	game->PerFrameConstants.View = game->gViewMat;
	game->PerFrameConstants.Proj = game->gProjMat;

	GameUpdatePerFrameConstants(game);

	game->RenderData.LightingData.CameraPos = game->Cam.CameraPos;
	GameUpdateConstantBuffer(game->DR->Context, sizeof(struct LightingData), &game->RenderData.LightingData, game->RenderData.PSConstBuffers[0]);
}

static void GameUpdateConstantBuffer(ID3D11DeviceContext* context,
	size_t bufferSize,
	void* data,
	ID3D11Buffer* dest)
{
	D3D11_MAPPED_SUBRESOURCE mapped = { 0 };

	if (FAILED(context->lpVtbl->Map(context, 
		(ID3D11Resource*)dest, 
		0, 
		D3D11_MAP_WRITE_DISCARD, 
		0, 
		&mapped)))
	{
		UtilsFatalError("ERROR: Failed to map constant buffer\n");
	}
	memcpy(mapped.pData, data, bufferSize);
	context->lpVtbl->Unmap(context, (ID3D11Resource*)dest, 0);
}

static void GameUpdatePerFrameConstants(Game* game)
{
	GameUpdateConstantBuffer(game->DR->Context, 
		sizeof(PerFrameConstants), 
		&game->PerFrameConstants,
		game->RenderData.VSConstBuffers[0]);
}

static struct Mesh* GameFindMeshByName(Game* game, const char* name, size_t* indexOffset)
{
	for (uint32_t i = 0; i < game->NumModels; ++i)
	{
		const struct Model* model = game->Models[i];
		for (uint32_t j = 0; j < model->NumMeshes; ++j)
		{
			struct Mesh* mesh = model->Meshes + j;
			if (strcmp(mesh->Name, name) == 0)
			{
				return mesh;
			}
			*indexOffset += mesh->NumFaces;
		}
	}
	return NULL;
}

static void GameRenderNew(Game* game)
{
	struct Renderer* r = &game->Renderer;

	RClear(r);

	RBindPixelShader(r, game->PhongPS);
	RBindVertexShader(r, game->VS);
	
	RBindConstantBuffers(r, BindTargets_VS, game->RenderData.VSConstBuffers, 1);
	RBindShaderResources(r, BindTargets_PS, game->RenderData.SRVs, TEXTURE_PULL);
	RBindConstantBuffers(r, BindTargets_PS, game->RenderData.PSConstBuffers, 1);

	for (size_t i = 0; i < game->m_NumActors; ++i)
	{
		const Actor* actor = game->m_Actors[i];

		game->PerFrameConstants.World = actor->m_World;
		GameUpdatePerFrameConstants(game);
		RBindConstantBuffers(r, BindTargets_VS, game->RenderData.VSConstBuffers, 1);

		RDrawIndexed(r, actor->m_IndexBuffer, actor->m_VertexBuffer, 
			sizeof(struct Vertex), 
			actor->m_NumIndices,
			0,
			0);
	}
	
	

	//// Light properties
	{
		const Vec3D scale = { 0.2f, 0.2f, 0.2f };
		Mat4X4 world = MathMat4X4ScaleFromVec3D(&scale);
		Mat4X4 translate = MathMat4X4TranslateFromVec3D(&game->RenderData.LightingData.PL.Position);
		game->PerFrameConstants.World = MathMat4X4MultMat4X4ByMat4X4(&world, &translate);
		GameUpdatePerFrameConstants(game);
	}

	RBindPixelShader(r, game->LightPS);
	RBindConstantBuffers(r, BindTargets_PS, game->RenderData.PSConstBuffers, 1);
	const Actor* sphere = game->m_Actors[2];
	RDrawIndexed(r, sphere->m_IndexBuffer, sphere->m_VertexBuffer, 
		sizeof(struct Vertex), 
		sphere->m_NumIndices, 
		0, 
		0);

	RPresent(r);
}

void GameTick(Game* game)
{
	TimerTick(&game->TickTimer);
	GameUpdate(game);
	GameRenderNew(game);
}

static void ErrorDescription(HRESULT hr)
{
	if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
	{
		hr = HRESULT_CODE(hr);
	}
	char* szErrMsg;

	if (FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&szErrMsg, 0, NULL) != 0)
	{
		OutputDebugStringA(szErrMsg);
		LocalFree(szErrMsg);
	}
	else
	{
		OutputDebugStringA("[Could not find a description for error # %#x.]\n");
	}
}

static void GameCreateConstantBuffer(ID3D11Device* device,
	size_t byteWidth,
	ID3D11Buffer** pDest)
{
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->lpVtbl->CreateBuffer(device, &bufferDesc, NULL, pDest)))
	{
		UtilsFatalError("ERROR: Failed to create per frame constants cbuffer\n");
	}
}

static void GameCreatePerFrameCB(Game* game)
{
	GameCreateConstantBuffer(game->DR->Device, sizeof(PerFrameConstants), &game->RenderData.VSConstBuffers[0]);
}

static void GameCreateSharedBuffers(Game* game)
{
	size_t numFaces = 0;
	for (uint32_t j = 0; j < game->NumModels; ++j)
	{
		for (uint32_t i = 0; i < game->Models[j]->NumMeshes; ++i)
		{
			struct Mesh* mesh = game->Models[j]->Meshes + i;
			numFaces += mesh->NumFaces;
		}
	}

	size_t bytes = sizeof(struct Vertex) * numFaces;
	struct Vertex* vertices = malloc(bytes);
	memset(vertices, 0, bytes);

	bytes = sizeof(uint32_t) * numFaces;
	uint32_t* indices = malloc(bytes);
	memset(indices, 0, bytes);

	size_t posOffs = 0;
	size_t normOffs = 0;
	size_t tcOffs = 0;

	for (uint32_t k = 0; k < game->NumModels; ++k)
	{
		struct Model* model = game->Models[k];
		for (uint32_t i = 0; i < model->NumMeshes; ++i)
		{
			const struct Mesh* mesh = model->Meshes + i;
			for (uint32_t j = 0; j < mesh->NumFaces; ++j)
			{
				const struct Face* face = model->Meshes[i].Faces + j;
				const struct Position* pos = mesh->Positions + face->posIdx - posOffs;
				const struct Normal* norm = mesh->Normals + face->normIdx - normOffs;
				const struct TexCoord* tc = mesh->TexCoords + face->texIdx - tcOffs;
				struct Vertex* vert = vertices + game->RenderData.NumIndices;

				vert->Position.X = pos->x;
				vert->Position.Y = pos->y;
				vert->Position.Z = pos->z;

				vert->Normal.X = norm->x;
				vert->Normal.Y = norm->y;
				vert->Normal.Z = norm->z;

				vert->TexCoords.X = tc->u;
				vert->TexCoords.Y = tc->v;

				indices[game->RenderData.NumIndices++] = j;

				game->RenderData.NumVertices++;
			}
			posOffs += mesh->NumPositions;
			normOffs += mesh->NumNormals;
			tcOffs += mesh->NumTexCoords;
		}
		posOffs = 0;
		normOffs = 0;
		tcOffs = 0;
	}
	assert(game->RenderData.NumVertices == numFaces);
	game->RenderData.Vertices = vertices;
	game->RenderData.Indices = indices;

	
}

void GameGenerateRandomOffsets(Game* game)
{
	const float a = MathRandom(0.0f, 2.0f);
	game->RenderData.MeshPositions = malloc(sizeof(Vec3D) * game->NumMeshes);
	memset(game->RenderData.MeshPositions, 0, sizeof(Vec3D) * game->NumMeshes);
	for (uint32_t i = 0; i < game->NumMeshes; ++i)
	{
		game->RenderData.MeshPositions[i].X = MathRandom(-10.0f + a * 10.0f, 10.0f + a * 10.0f);
		game->RenderData.MeshPositions[i].Y = MathRandom(-10.0f + a * 10.0f, 10.0f + a * 10.0f);
		game->RenderData.MeshPositions[i].Z = MathRandom(-10.0f + a * 10.0f, 10.0f + a * 10.0f);
	}
}

static void GameCreatePixelShader(const char* filepath, ID3D11Device* device, ID3D11PixelShader** ps)
{
	unsigned int bufferSize = 0;
	unsigned char* bytes = UtilsReadData(filepath, &bufferSize);

	if (FAILED(device->lpVtbl->CreatePixelShader(device, bytes, bufferSize, NULL, ps)))
	{
		UTILS_FATAL_ERROR("Failed to create pixel shader from %s", filepath);
	}

	free(bytes);
}

static void GameCreateInputLayout(ID3D11Device* device, ID3D11InputLayout** il, unsigned char* bytes, size_t bufferSize)
{
	const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"NORMAL",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				sizeof(float) * 3,
				0,
			},
			{
				"TEXCOORDS",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				sizeof(float) * 3 * 2,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
	};

	if (FAILED(device->lpVtbl->CreateInputLayout(device, inputElementDesc, sizeof(inputElementDesc) / sizeof(*inputElementDesc), bytes, bufferSize, il)))
	{
		UtilsFatalError("Failed to create input layout");
	}
}

static void GameCreateVertexShader(const char* filepath, ID3D11Device* device, ID3D11VertexShader** vs, ID3D11InputLayout** il)
{
	unsigned int bufferSize = 0;
	unsigned char* bytes = UtilsReadData(filepath, &bufferSize);

	if (FAILED(device->lpVtbl->CreateVertexShader(device, bytes, bufferSize, NULL, vs)))
	{
		UTILS_FATAL_ERROR("Failed to create vertex shader from %s", filepath);
	}
	GameCreateInputLayout(device, il, bytes, bufferSize);
	free(bytes);
}

static void GameCreateVertexBuffer(const void* vertexData, const uint32_t numVertices, ID3D11Device* device, ID3D11Buffer** vb)
{
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = vertexData;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(struct Vertex) * (uint32_t)numVertices;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(struct Vertex);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	if (FAILED(device->lpVtbl->CreateBuffer(device, &bufferDesc, &subresourceData, vb)))
	{
		OutputDebugStringA("ERROR: Failed to create vertex buffer\n");
		ExitProcess(EXIT_FAILURE);
	}
}

static void GameCreateIndexBuffer(const void* indexData, const uint32_t numIndices, ID3D11Device* device, ID3D11Buffer** ib)
{
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = indexData;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(uint32_t) * numIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	if (FAILED(device->lpVtbl->CreateBuffer(device, &bufferDesc, &subresourceData, ib)))
	{
		OutputDebugStringA("ERROR: Failed to create index buffer\n");
		ExitProcess(EXIT_FAILURE);
	}
}

static void RenderDataSetSRV(struct RenderData* rd)
{
	rd->SRVs[0] = rd->DefaultTexture.SRV;
	rd->SRVs[1] = rd->SpecularTexture.SRV;
	rd->SRVs[2] = rd->GlossTexture.SRV;
	rd->SRVs[3] = rd->NormalTexture.SRV;
}

static void GameCreateActors(Game* game)
{
	const char* models[] = {
		"assets/meshes/rocket.obj",
		"assets/meshes/cube.obj",
		"assets/meshes/sphere.obj",
		"assets/meshes/bunny.obj",
	};

	const float scales[] = {
		1.0f,
		1.0f,
		1.0f,
		0.01f,
	};

	const Vec3D rotations[] = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, MathToRadians(180.0f), 0.0f},
	};

	const Vec3D offsets[] = {
		{MathRandom(-10.0f, 10.0f), 0.0f, MathRandom(-10.0f, 10.0f)},
		{MathRandom(-10.0f, 10.0f), 0.0f, MathRandom(-10.0f, 10.0f)},
		{MathRandom(-10.0f, 10.0f), 0.0f, MathRandom(-10.0f, 10.0f)},
		{MathRandom(-10.0f, 10.0f), 0.0f, MathRandom(-10.0f, 10.0f)},
	};

	game->m_Actors = realloc(game->m_Actors, 
		sizeof(Actor*) * _countof(models));
	assert(game->m_Actors);

	for (size_t i = 0; i < _countof(models); ++i)
	{
		Actor* actor = ActorNew();
		ActorInit(actor);
		ActorLoadModel(actor, models[i]);
		ActorCreateIndexBuffer(actor, game->DR->Device);
		ActorCreateVertexBuffer(actor, game->DR->Device);
		ActorScale(actor, scales[i]);
		ActorRotate(actor, rotations[i].X, rotations[i].Y, rotations[i].Z);
		ActorTranslate(actor, offsets[i]);

		game->m_Actors[game->m_NumActors++] = actor;
	}
}

void GameInitialize(Game* game, HWND hWnd, int width, int height)
{
#ifdef MATH_TEST
	MathTest();
#endif
	DRSetWindow(game->DR, hWnd, width, height);
	DRCreateDeviceResources(game->DR);
	DRCreateWindowSizeDependentResources(game->DR);
	TimerInitialize(&game->TickTimer);
	KeyboardInit(&game->Keyboard);
	MouseInit(&game->Mouse, game->DR->BackbufferWidth, game->DR->BackbufferHeight);
	const Vec3D cameraPos = { 0.0f, 0.0f, -5.0f };
	CameraInit(&game->Cam, &cameraPos, &game->Keyboard, &game->Mouse);
	RenderDataInit(&game->RenderData, &cameraPos);

	// init actors
	GameCreateActors(game);

	GameLoadTextureFromFile(game->DR, "assets/textures/BricksFlemishRed001_COL_VAR1_1K.jpg", &game->RenderData.DefaultTexture);
	GameLoadTextureFromFile(game->DR, "assets/textures/BricksFlemishRed001_REFL_1K.jpg", &game->RenderData.SpecularTexture);
	GameLoadTextureFromFile(game->DR, "assets/textures/BricksFlemishRed001_GLOSS_1K.jpg", &game->RenderData.GlossTexture);
	GameLoadTextureFromFile(game->DR, "assets/textures/BricksFlemishRed001_NRM_1K.png", &game->RenderData.NormalTexture);
	//GameCreateSharedBuffers(game);
	GameGenerateRandomOffsets(game);
	ID3D11Device1* device = game->DR->Device;
	game->gWorldMat = MathMat4X4Identity();
	game->gProjMat = MathMat4X4Identity();
	game->gViewMat = MathMat4X4Identity();

	GameCreatePixelShader("PixelShader.cso", (ID3D11Device*)device, &game->PS);
	GameCreatePixelShader("PhongPS.cso", (ID3D11Device*)device, &game->PhongPS);
	GameCreatePixelShader("LightPS.cso", (ID3D11Device*)device, &game->LightPS);
	GameCreateVertexShader("VertexShader.cso", (ID3D11Device*)device, &game->VS, &game->InputLayout);

	GameCreatePerFrameCB(game);
	GameCreateConstantBuffer(game->DR->Device, sizeof(struct LightingData), &game->RenderData.PSConstBuffers[0]);
	GameCreateDefaultSampler(game);

	RInit(&game->Renderer);
	RSetDeviceResources(&game->Renderer, game->DR);
	RSetPrimitiveTopology(&game->Renderer, R_DEFAULT_PRIMTIVE_TOPOLOGY);
	RSetRasterizerState(&game->Renderer, game->DR->RasterizerState);
	RSetInputLayout(&game->Renderer, game->InputLayout);
	RSetSamplerState(&game->Renderer, game->DefaultSampler);
	RenderDataSetSRV(&game->RenderData);
}

void GameGetDefaultSize(Game* game, int* width, int* height)
{
	*width = DEFAULT_WIN_WIDTH;
	*height = DEFAULT_WIN_HEIGHT;
}

void GameOnKeyDown(Game* game, WPARAM key)
{
	KeyboardOnKeyDown(&game->Keyboard, key);

	if (key == VK_ESCAPE)
	{
		PostQuitMessage(0);
	}
}

void GameOnKeyUp(Game* game, WPARAM key)
{
	KeyboardOnKeyUp(&game->Keyboard, key);
}

void GameOnMouseMove(Game* game, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	MouseOnMouseMove(&game->Mouse, message, wParam, lParam);
}

void GameLoadTextureFromFile(DeviceResources* dr, const char* filename, struct Texture* texture)
{
	int width = 0;
	int height = 0;
	int channelsInFile = 0;
	const int desiredChannels = 4;

	unsigned char* bytes = stbi_load(filename, &width, &height, &channelsInFile, desiredChannels);
	if (!bytes)
	{
		UtilsDebugPrint("ERROR: Failed to load texture from %s\n", filename);
		ExitProcess(EXIT_FAILURE);
	}

	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
		subresourceData.pSysMem = bytes;
		subresourceData.SysMemPitch = width * sizeof(unsigned char) * desiredChannels;

		if (FAILED(dr->Device->lpVtbl->CreateTexture2D(dr->Device, &desc, &subresourceData, &texture->Resource)))
		{
			UtilsDebugPrint("ERROR: Failed to create texture from file %s\n", filename);
			ExitProcess(EXIT_FAILURE);
		}
	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { 0 };
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		if (FAILED(dr->Device->lpVtbl->CreateShaderResourceView(dr->Device, (ID3D11Resource*)texture->Resource, &srvDesc, &texture->SRV)))
		{
			UtilsDebugPrint("ERROR: Failed to create SRV from file %s\n", filename);
			ExitProcess(EXIT_FAILURE);
		}

		dr->Context->lpVtbl->GenerateMips(dr->Context, texture->SRV);
	}

	stbi_image_free(bytes);
}