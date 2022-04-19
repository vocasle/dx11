#include "Game.h"
#include "Utils.h"
#include "Math.h"
#include "Camera.h"
#include "MeshGenerator.h"

#include <windowsx.h>
#if _DEBUG
#include <vld.h>
#endif

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

static void GameInitPerSceneConstants(Game* game)
{
	struct PointLight pl = { 0 };
	const Vec3D positions[] = {
		{-4.0f, 2.0f, -4.0f},
		{-4.0f, 2.0f, 4.0f},
		{0.0f, 5.5f, 0.0f},
		{4.0f, 2.0f, -4.0f},
	};

	const Color colors[4][3] = {
		{ {0.1f, 0.1f, 0.1f, 1.0f}, {0.8f, 0.1f, 0.1f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.1f, 0.1f, 0.1f, 1.0f}, {0.1f, 0.8f, 0.1f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.1f, 0.1f, 0.1f, 1.0f}, {0.1f, 0.1f, 0.8f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.1f, 0.1f, 0.1f, 1.0f}, {0.8f, 0.1f, 0.8f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
	};

	for (uint32_t i = 0; i < 4; ++i)
	{
		pl.Position = positions[i];
		pl.Ambient = colors[i][0];
		pl.Diffuse = colors[i][1];
		pl.Specular = colors[i][2];
		pl.Att = MathVec3DFromXYZ(1.0f, 0.7f, 1.8f);
		pl.Range = MathRandom(3.0f, 5.0f);
		game->m_PerSceneData.pointLights[i] = pl;
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
	COM_FREE(game->m_PerFrameCB);
	COM_FREE(game->m_PerObjectCB);
	COM_FREE(game->m_PerSceneCB);
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
void GameUpdate(Game* game)
{
	CameraUpdatePos(&game->Cam, game->TickTimer.DeltaMillis);
	CameraProcessMouse(&game->Cam, game->TickTimer.DeltaMillis);

	const float width = (float)game->DR->BackbufferWidth;
	const float height = (float)game->DR->BackbufferHeight;

	game->gViewMat = CameraGetViewMat(&game->Cam);
	game->gProjMat = MathMat4X4PerspectiveFov(MathToRadians(45.0f), width / height, 0.1f, 100.0f);
	
	game->m_PerFrameData.view = game->gViewMat;
	game->m_PerFrameData.proj = game->gProjMat;
	game->m_PerFrameData.cameraPosW = game->Cam.CameraPos;

	GameUpdateConstantBuffer(game->DR->Context, sizeof(PerFrameConstants), &game->m_PerFrameData, game->m_PerFrameCB);
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
	
	RBindConstantBuffer(r, BindTargets_VS, game->m_PerFrameCB, 1);
	RBindConstantBuffer(r, BindTargets_PS, game->m_PerFrameCB, 1);

	RBindConstantBuffer(r, BindTargets_VS, game->m_PerSceneCB, 2);
	RBindConstantBuffer(r, BindTargets_PS, game->m_PerSceneCB, 2);

	for (size_t i = 0; i < game->m_NumActors; ++i)
	{
		const Actor* actor = game->m_Actors[i];
		RBindShaderResources(r, BindTargets_PS, actor->m_Textures, ACTOR_NUM_TEXTURES);
		game->m_PerObjectData.world = actor->m_World;
		GameUpdateConstantBuffer(game->DR->Context,
			sizeof(PerObjectConstants),
			&game->m_PerObjectData,
			game->m_PerObjectCB);
		RBindConstantBuffer(r, BindTargets_VS, game->m_PerObjectCB, 0);
		RBindConstantBuffer(r, BindTargets_PS, game->m_PerObjectCB, 0);

		RDrawIndexed(r, actor->m_IndexBuffer, actor->m_VertexBuffer, 
			sizeof(struct Vertex), 
			actor->m_NumIndices,
			0,
			0);
	}
	
	//// Light properties
	for (uint32_t i = 0; i < _countof(game->m_PerSceneData.pointLights); ++i)
	{
		const Vec3D scale = { 0.2f, 0.2f, 0.2f };
		Mat4X4 world = MathMat4X4ScaleFromVec3D(&scale);
		Mat4X4 translate = MathMat4X4TranslateFromVec3D(&game->m_PerSceneData.pointLights[i].Position);
		game->m_PerObjectData.world = MathMat4X4MultMat4X4ByMat4X4(&world, &translate);
		GameUpdateConstantBuffer(game->DR->Context,
			sizeof(PerObjectConstants),
			&game->m_PerObjectData,
			game->m_PerObjectCB);

		RBindPixelShader(r, game->LightPS);
		RBindConstantBuffer(r, BindTargets_VS, game->m_PerObjectCB, 0);
		RBindConstantBuffer(r, BindTargets_PS, game->m_PerObjectCB, 0);
		const Actor* sphere = game->m_Actors[2];
		RDrawIndexed(r, sphere->m_IndexBuffer, sphere->m_VertexBuffer,
			sizeof(struct Vertex),
			sphere->m_NumIndices,
			0,
			0);
	}

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

static void GameCreateActors(Game* game)
{
	const char* models[] = {
		"assets/meshes/rocket.obj",
		"assets/meshes/cube.obj",
		"assets/meshes/sphere.obj",
		"assets/meshes/bunny.obj",
	};

	const char* diffuseTextures[] = {
		"assets/textures/drywall_diffuse.jpg",
		"assets/textures/bricks_diffuse.jpg",
		"assets/textures/cliff_diffuse.jpg",
		"assets/textures/marble_diffuse.jpg",
	};

	const char* specularTextures[] = {
		"assets/textures/drywall_reflection.jpg",
		"assets/textures/bricks_reflection.jpg",
		"assets/textures/cliff_reflection.jpg",
		"assets/textures/marble_reflection.jpg",
	};

	const char* normalTextures[] = {
		"assets/textures/drywall_normal.png",
		"assets/textures/bricks_normal.png",
		"assets/textures/cliff_normal.jpg",
		"assets/textures/marble_normal.png",
	};

	const char* glossTextures[] = {
		"assets/textures/drywall_gloss.jpg",
		"assets/textures/bricks_gloss.jpg",
		"assets/textures/cliff_gloss.jpg",
		"assets/textures/marble_gloss.jpg",
	};

	const float scales[] = {
		1.0f,
		1.0f,
		1.0f,
		0.008f,
	};

	const Vec3D rotations[] = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, MathToRadians(180.0f), 0.0f},
	};
	const Vec3D offsets[] = {
		{0.0f, 0.0f, 0.0f},
		{4.0f, 0.0f, -4.0f},
		{-4.0f, 0.0f, 4.0f},
		{0.0, 1.0f, 2.5f},
	};

	const Material material = {
		// 0.24725 	0.1995 	0.0745 	0.75164 	0.60648 	0.22648 	0.628281 	0.555802 	0.366065 	0.4
		{0.24725f, 0.1995f, 0.0745f, 1.0f},
		{0.75164f, 0.60648f, 0.22648f, 1.0f},
		{0.628281f, 0.555802f, 0.366065f, 0.4f}
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
		ActorLoadTexture(actor, diffuseTextures[i], TextureType_Diffuse, game->DR->Device, game->DR->Context);
		ActorLoadTexture(actor, specularTextures[i], TextureType_Specular, game->DR->Device, game->DR->Context);
		ActorLoadTexture(actor, glossTextures[i], TextureType_Gloss, game->DR->Device, game->DR->Context);
		ActorLoadTexture(actor, normalTextures[i], TextureType_Normal, game->DR->Device, game->DR->Context);
		ActorSetMaterial(actor, &material);

		game->m_Actors[game->m_NumActors++] = actor;
	}

	game->m_Actors = realloc(game->m_Actors, sizeof(Actor*) * 6);
	{
		const Vec3D origin = { 0.0f, 0.0f, 0.0f };
		struct Mesh* mesh = MGGeneratePlane(&origin, 10.0f, 10.0f);
		Actor* plane = ActorFromMesh(mesh);
		MeshFree(mesh);
		ActorCreateIndexBuffer(plane, game->DR->Device);
		ActorCreateVertexBuffer(plane, game->DR->Device);
		const Vec3D offset = { 0.0f, -1.0f, 0.0f };
		ActorTranslate(plane, offset);
		ActorLoadTexture(plane, "assets/textures/chess.jpg", TextureType_Diffuse, game->DR->Device, game->DR->Context);
		game->m_Actors[game->m_NumActors++] = plane;
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
	GameInitPerSceneConstants(game);

	// init actors
	GameCreateActors(game);

	ID3D11Device1* device = game->DR->Device;
	game->gWorldMat = MathMat4X4Identity();
	game->gProjMat = MathMat4X4Identity();
	game->gViewMat = MathMat4X4Identity();

	GameCreatePixelShader("PixelShader.cso", (ID3D11Device*)device, &game->PS);
	GameCreatePixelShader("PhongPS.cso", (ID3D11Device*)device, &game->PhongPS);
	GameCreatePixelShader("LightPS.cso", (ID3D11Device*)device, &game->LightPS);
	GameCreateVertexShader("VertexShader.cso", (ID3D11Device*)device, &game->VS, &game->InputLayout);

	GameCreateConstantBuffer(game->DR->Device, sizeof(PerSceneConstants), &game->m_PerSceneCB);
	GameCreateConstantBuffer(game->DR->Device, sizeof(PerObjectConstants), &game->m_PerObjectCB);
	GameCreateConstantBuffer(game->DR->Device, sizeof(PerFrameConstants), &game->m_PerFrameCB);
	GameUpdateConstantBuffer(game->DR->Context, sizeof(PerSceneConstants), &game->m_PerSceneData, game->m_PerSceneCB);

	GameCreateDefaultSampler(game);

	RInit(&game->Renderer);
	RSetDeviceResources(&game->Renderer, game->DR);
	RSetPrimitiveTopology(&game->Renderer, R_DEFAULT_PRIMTIVE_TOPOLOGY);
	RSetRasterizerState(&game->Renderer, game->DR->RasterizerState);
	RSetInputLayout(&game->Renderer, game->InputLayout);
	RSetSamplerState(&game->Renderer, game->DefaultSampler);
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