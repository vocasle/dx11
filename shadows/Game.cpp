#include "Game.h"
#include "Utils.h"
#include "Math.h"
#include "Camera.h"
#include "MeshGenerator.h"

static void GameUpdateConstantBuffer(ID3D11DeviceContext* context,
	size_t bufferSize,
	void* data,
	ID3D11Buffer* dest)
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	if (FAILED(context->Map((ID3D11Resource*)dest,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped)))
	{
		UtilsFatalError("ERROR: Failed to map constant buffer\n");
	}
	memcpy(mapped.pData, data, bufferSize);
	context->Unmap((ID3D11Resource*)dest, 0);
}

static void GameCreateConstantBuffer(ID3D11Device* device,
	size_t byteWidth,
	ID3D11Buffer** pDest)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&bufferDesc, NULL, pDest)))
	{
		UtilsFatalError("ERROR: Failed to create per frame constants cbuffer\n");
	}
}

static void GameCreatePixelShader(const char* filepath, ID3D11Device* device, ID3D11PixelShader** ps)
{
	unsigned int bufferSize = 0;
	unsigned char* bytes = UtilsReadData(filepath, &bufferSize);

	if (FAILED(device->CreatePixelShader(bytes, bufferSize, NULL, ps)))
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
				12,
				D3D11_INPUT_PER_VERTEX_DATA,
				0,
			},
			{
				"TANGENT",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				24,
				D3D11_INPUT_PER_VERTEX_DATA,
				0,
			},
			{
				"BITANGENT",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				36,
				D3D11_INPUT_PER_VERTEX_DATA,
				0,
			},
			{
				"TEXCOORDS",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				48,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
	};

	if (FAILED(device->CreateInputLayout(inputElementDesc, sizeof(inputElementDesc) / sizeof(*inputElementDesc), bytes, bufferSize, il)))
	{
		UtilsFatalError("Failed to create input layout");
	}
}

static void GameCreateVertexShader(const char* filepath, ID3D11Device* device, ID3D11VertexShader** vs, ID3D11InputLayout** il)
{
	unsigned int bufferSize = 0;
	unsigned char* bytes = UtilsReadData(filepath, &bufferSize);

	if (FAILED(device->CreateVertexShader(bytes, bufferSize, NULL, vs)))
	{
		UTILS_FATAL_ERROR("Failed to create vertex shader from %s", filepath);
	}
	GameCreateInputLayout(device, il, bytes, bufferSize);
	free(bytes);
}

static void GameCreateVertexBuffer(const void* vertexData, const uint32_t numVertices, ID3D11Device* device, ID3D11Buffer** vb)
{
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = vertexData;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Vertex) * (uint32_t)numVertices;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(Vertex);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	if (FAILED(device->CreateBuffer(&bufferDesc, &subresourceData, vb)))
	{
		OutputDebugStringA("ERROR: Failed to create vertex buffer\n");
		ExitProcess(EXIT_FAILURE);
	}
}

static void GameCreateIndexBuffer(const void* indexData, const uint32_t numIndices, ID3D11Device* device, ID3D11Buffer** ib)
{
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = indexData;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(uint32_t) * numIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	if (FAILED(device->CreateBuffer(&bufferDesc, &subresourceData, ib)))
	{
		OutputDebugStringA("ERROR: Failed to create index buffer\n");
		ExitProcess(EXIT_FAILURE);
	}
}

void Game::CreateDefaultSampler()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(m_DR->GetDevice()->CreateSamplerState(&samplerDesc, m_DefaultSampler.ReleaseAndGetAddressOf())))
	{
		UtilsDebugPrint("ERROR: Failed to create default sampler state\n");
		ExitProcess(EXIT_FAILURE);
	}
}

void Game::InitPerSceneConstants()
{
	struct PointLight pl = {};
	const Vec3D positions[] = {
		{-4.0f, 1.5f, -4.0f},
		{-4.0f, 1.5f, 4.0f},
		{4.0f, 1.5f, 4.0f},
		{4.0f, 1.5f, -4.0f},
	};

	for (uint32_t i = 0; i < 4; ++i)
	{
		pl.Position = positions[i];
		pl.Ambient = ColorFromRGBA(0.03f, 0.03f, 0.03f, 1.0f);
		pl.Diffuse = ColorFromRGBA(0.2f, 0.2f, 0.2f, 1.0f);
		pl.Specular = ColorFromRGBA(0.2f, 0.2f, 0.2f, 1.0f);
		pl.Att = MathVec3DFromXYZ(1.0f, 0.09f, 0.032f);
		pl.Range = 5.0f;
		m_PerSceneData.pointLights[i] = pl;
	}

	DirectionalLight dirLight = {};
	dirLight.Ambient = ColorFromRGBA(0.02f, 0.02f, 0.02f, 1.0f);
	dirLight.Diffuse = ColorFromRGBA(0.7f, 0.7f, 0.6f, 1.0f);
	dirLight.Specular = ColorFromRGBA(0.8f, 0.8f, 0.7f, 1.0f);
	dirLight.Direction = MathVec3DFromXYZ(5.0f / sqrtf(50.0f), -5.0f / sqrtf(50.0f), 0.0f);
	Mat4X4 rotMat = MathMat4X4RotateY(MathToRadians(90.0f));
	Vec4D direction = { dirLight.Direction, 1.0f };
	direction = MathMat4X4MultVec4DByMat4X4(&direction, &rotMat);
	dirLight.Direction = { direction.X, direction.Y, direction.Z };
	m_PerSceneData.dirLight = dirLight;

	SpotLight spotLight = {};
	spotLight.Position = m_Camera.GetPos();
	spotLight.Direction = m_Camera.GetAt();
	spotLight.Ambient = ColorFromRGBA(0.0f, 0.0f, 0.0f, 1.0f);
	spotLight.Diffuse = ColorFromRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	spotLight.Specular = ColorFromRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	spotLight.Att = MathVec3DFromXYZ(1.0f, 0.09f, 0.032f);
	spotLight.Range = 5.0f;
	m_PerSceneData.spotLights[0] = spotLight;
}

Game::Game():
	m_Camera{ {0.0f, 0.0f, -5.0f} }
{
	m_DR = std::make_unique<DeviceResources>();
}

Game::~Game()
{
}

void Game::Clear()
{
	ID3D11DeviceContext* ctx = m_DR->GetDeviceContext();
	ID3D11RenderTargetView* rtv = m_DR->GetRenderTargetView();
	ID3D11DepthStencilView* dsv = m_DR->GetDepthStencilView();

	static const float CLEAR_COLOR[4] = {0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f};

	ctx->Flush();

	ctx->ClearRenderTargetView(rtv, CLEAR_COLOR);
	ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ctx->OMSetRenderTargets(1, &rtv, dsv);
	ctx->RSSetViewports(1, &m_DR->GetViewport());
}

void Game::Update()
{
	m_Camera.UpdatePos(m_Timer.DeltaMillis);
	m_Camera.ProcessMouse(m_Timer.DeltaMillis);
	
	m_PerFrameData.view = m_Camera.GetViewMat();
	m_PerFrameData.proj = m_Camera.GetProjMat();
	m_PerFrameData.cameraPosW = m_Camera.GetPos();

	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());

	BuildShadowTransform();

	// update directional light
	static float elapsedTime = 0.0f;
	elapsedTime += (float)m_Timer.DeltaMillis / 1000.0f;
	m_PerSceneData.dirLight.Direction.X = sinf(elapsedTime);
	m_PerSceneData.dirLight.Direction.Y = 0.5f;
	m_PerSceneData.dirLight.Direction.Z = cosf(elapsedTime);
	MathVec3DNormalize(&m_PerSceneData.dirLight.Direction);

	//m_PerSceneData.spotLights[0].Position = m_Camera.CameraPos;
	//m_PerSceneData.spotLights[0].Direction = m_Camera.FocusPoint;
	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerSceneConstants), &m_PerSceneData, m_PerSceneCB.Get());
}

void Game::Render()
{

	m_ShadowMap.Bind(m_DR->GetDeviceContext());
	{
		m_Renderer.BindVertexShader(m_VS.Get());
		m_Renderer.BindPixelShader(nullptr);
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerSceneCB.Get(), 2);

		for (size_t i = 0; i < m_Actors.size(); ++i)
		{
			const Actor& actor = m_Actors[i];
			m_PerObjectData.world = actor.GetWorld();
			m_PerObjectData.material = actor.GetMaterial();
			GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
				sizeof(PerObjectConstants),
				&m_PerObjectData,
				m_PerObjectCB.Get());
			m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
			m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);

			m_Renderer.DrawIndexed(actor.GetIndexBuffer(), actor.GetVertexBuffer(),
				sizeof(Vertex),
				actor.GetNumIndices(),
				0,
				0);
		}
	}
	m_ShadowMap.Unbind(m_DR->GetDeviceContext());
	// reset view proj matrix back to camera
	{
		m_PerFrameData.view = m_Camera.GetViewMat();
		m_PerFrameData.proj = m_Camera.GetProjMat();
		m_PerFrameData.cameraPosW = m_Camera.GetPos();
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());
	}

	m_Renderer.Clear();

	m_Renderer.BindPixelShader(m_PhongPS.Get());
	m_Renderer.BindVertexShader(m_VS.Get());

	
	m_Renderer.BindShaderResource(BindTargets::PixelShader, m_ShadowMap.GetDepthMapSRV(), 4);
	m_Renderer.SetSamplerState(m_ShadowMap.GetShadowSampler(), 1);
	
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
	m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerFrameCB.Get(), 1);

	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerSceneCB.Get(), 2);
	m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerSceneCB.Get(), 2);

	for (size_t i = 0; i < m_Actors.size(); ++i)
	{
		const Actor& actor = m_Actors[i];
		m_Renderer.BindShaderResources(BindTargets::PixelShader, actor.GetShaderResources(), ACTOR_NUM_TEXTURES);
		m_PerObjectData.world = actor.GetWorld();
		m_PerObjectData.material = actor.GetMaterial();
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
			sizeof(PerObjectConstants),
			&m_PerObjectData,
			m_PerObjectCB.Get());
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);

		m_Renderer.DrawIndexed(actor.GetIndexBuffer(), actor.GetVertexBuffer(),
			sizeof(Vertex),
			actor.GetNumIndices(),
			0,
			0);
	}
	
	//// Light properties
	for (uint32_t i = 0; i < _countof(m_PerSceneData.pointLights); ++i)
	{
		const Vec3D scale = { 0.2f, 0.2f, 0.2f };
		Mat4X4 world = MathMat4X4ScaleFromVec3D(&scale);
		Mat4X4 translate = MathMat4X4TranslateFromVec3D(&m_PerSceneData.pointLights[i].Position);
		m_PerObjectData.world = MathMat4X4MultMat4X4ByMat4X4(&world, &translate);
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
			sizeof(PerObjectConstants),
			&m_PerObjectData,
			m_PerObjectCB.Get());

		m_Renderer.BindPixelShader(m_LightPS.Get());
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);
		const Actor& sphere = m_Actors[2];
		m_Renderer.DrawIndexed(sphere.GetIndexBuffer(),
			sphere.GetVertexBuffer(),
			sizeof(struct Vertex),
			sphere.GetNumIndices(),
			0,
			0);
	}

	m_Renderer.Present();
}

void Game::Tick()
{
	TimerTick(&m_Timer);
	Update();
	Render();
}

void Game::CreateActors()
{
	const std::string models[] = {
		UtilsFormatStr("%s/meshes/rocket.obj", ASSETS_ROOT),
		UtilsFormatStr("%s/meshes/cube.obj", ASSETS_ROOT),
		UtilsFormatStr("%s/meshes/sphere.obj", ASSETS_ROOT),
	};

	const std::string diffuseTextures[] = {
		UtilsFormatStr("%s/textures/drywall_diffuse.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/bricks_diffuse.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/cliff_diffuse.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/marble_diffuse.jpg", ASSETS_ROOT),
	};

	const std::string specularTextures[] = {
		UtilsFormatStr("%s/textures/drywall_reflection.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/bricks_reflection.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/cliff_reflection.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/marble_reflection.jpg", ASSETS_ROOT),
	};

	const std::string normalTextures[] = {
		UtilsFormatStr("%s/textures/drywall_normal.png", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/bricks_normal.png", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/cliff_normal.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/marble_normal.png", ASSETS_ROOT),
	};

	const std::string glossTextures[] = {
		UtilsFormatStr("%s/textures/drywall_gloss.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/bricks_gloss.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/cliff_gloss.jpg", ASSETS_ROOT),
		UtilsFormatStr("%s/textures/marble_gloss.jpg", ASSETS_ROOT),
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
		{0.24725f, 0.1995f, 0.0745f, 1.0f},
		{0.75164f, 0.60648f, 0.22648f, 1.0f},
		{0.628281f, 0.555802f, 0.366065f, 0.4f}
	};

	for (size_t i = 0; i < _countof(models); ++i)
	{
		Actor actor = Actor();
		actor.LoadModel(models[i].c_str());
		actor.CreateIndexBuffer(m_DR->GetDevice());
		actor.CreateVertexBuffer(m_DR->GetDevice());
		actor.Scale(scales[i]);
		actor.Rotate(rotations[i].X, rotations[i].Y, rotations[i].Z);
		actor.Translate(offsets[i]);
		actor.LoadTexture(diffuseTextures[i].c_str(), TextureType::Diffuse, m_DR->GetDevice(), m_DR->GetDeviceContext());
		actor.LoadTexture(specularTextures[i].c_str(), TextureType::Specular, m_DR->GetDevice(), m_DR->GetDeviceContext());
		actor.LoadTexture(glossTextures[i].c_str(), TextureType::Gloss, m_DR->GetDevice(), m_DR->GetDeviceContext());
		actor.LoadTexture(normalTextures[i].c_str(), TextureType::Normal, m_DR->GetDevice(), m_DR->GetDeviceContext());
		actor.SetMaterial(&material);

		m_Actors.emplace_back(actor);
	}

	{
		const Vec3D origin = { 0.0f, 0.0f, 0.0f };
		std::unique_ptr<Mesh> mesh = MGGeneratePlane(&origin, 10.0f, 10.0f);
		Actor plane = Actor(mesh.get());
		plane.CreateIndexBuffer(m_DR->GetDevice());
		plane.CreateVertexBuffer(m_DR->GetDevice());
		const Vec3D offset = { 0.0f, -1.0f, 0.0f };
		plane.Translate(offset);
		plane.LoadTexture(UtilsFormatStr("%s/textures/drywall_diffuse.jpg", ASSETS_ROOT).c_str(), TextureType::Diffuse, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/drywall_normal.png", ASSETS_ROOT).c_str(), TextureType::Normal, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/drywall_gloss.jpg", ASSETS_ROOT).c_str(), TextureType::Gloss, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/drywall_reflection.jpg", ASSETS_ROOT).c_str(), TextureType::Specular, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.SetMaterial(&material);
		m_Actors.emplace_back(plane);
	}
}

void Game::Initialize(HWND hWnd, uint32_t width, uint32_t height)
{
#ifdef MATH_TEST
	MathTest();
#endif
	m_DR->SetWindow(hWnd, width, height);
	m_DR->CreateDeviceResources();
	m_DR->CreateWindowSizeDependentResources();
	TimerInitialize(&m_Timer);
	Mouse::Get().SetWindowDimensions(m_DR->GetBackBufferWidth(), m_DR->GetBackBufferHeight());
	m_ShadowMap.InitResources(m_DR->GetDevice(), 2048, 2048);
	m_Camera.SetViewDimensions(m_DR->GetBackBufferWidth(), m_DR->GetBackBufferHeight());

	// init actors
	CreateActors();
	InitPerSceneConstants();

	ID3D11Device* device = m_DR->GetDevice();

	GameCreatePixelShader("PixelShader.cso", (ID3D11Device*)device, m_PS.ReleaseAndGetAddressOf());
	GameCreatePixelShader("PhongPS.cso", (ID3D11Device*)device, m_PhongPS.ReleaseAndGetAddressOf());
	GameCreatePixelShader("LightPS.cso", (ID3D11Device*)device, m_LightPS.ReleaseAndGetAddressOf());
	GameCreateVertexShader("VertexShader.cso", (ID3D11Device*)device, m_VS.ReleaseAndGetAddressOf(), m_InputLayout.ReleaseAndGetAddressOf());

	GameCreateConstantBuffer(m_DR->GetDevice(), sizeof(PerSceneConstants), &m_PerSceneCB);
	GameCreateConstantBuffer(m_DR->GetDevice(), sizeof(PerObjectConstants), &m_PerObjectCB);
	GameCreateConstantBuffer(m_DR->GetDevice(), sizeof(PerFrameConstants), &m_PerFrameCB);
	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerSceneConstants), &m_PerSceneData, m_PerSceneCB.Get());

	CreateDefaultSampler();

	m_Renderer.SetDeviceResources(m_DR.get());
	m_Renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
	m_Renderer.SetRasterizerState(m_DR->GetRasterizerState());
	m_Renderer.SetInputLayout(m_InputLayout.Get());
	m_Renderer.SetSamplerState(m_DefaultSampler.Get(), 0);
}

void Game::GetDefaultSize(uint32_t* width, uint32_t* height)
{
	*width = DEFAULT_WIN_WIDTH;
	*height = DEFAULT_WIN_HEIGHT;
}

void Game::OnKeyDown(WPARAM key)
{
	Keyboard::Get().OnKeyDown(key);

	if (key == VK_ESCAPE)
	{
		PostQuitMessage(0);
	}
}

void Game::OnKeyUp(WPARAM key)
{
	Keyboard::Get().OnKeyUp(key);
}

void Game::OnMouseMove(uint32_t message, WPARAM wParam, LPARAM lParam)
{
	Mouse::Get().OnMouseMove(message, wParam, lParam);
}

void Game::BuildShadowTransform()
{
	struct BoundingSphere
	{
		BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
		Vec3D Center;
		float Radius;
	};

	BoundingSphere sceneBounds = {};
	sceneBounds.Center = { 0.0f, 0.0f, 0.0f };
	sceneBounds.Radius = 10.0f;

	// Only the first "main" light casts a shadow.
	Vec3D lightDir = m_PerSceneData.dirLight.Direction;
	Vec3D lightPos = MathVec3DModulateByScalar(&lightDir, -2.0f * sceneBounds.Radius);
	Vec3D targetPos = { 0.0f, 0.0f, 0.0f };
	Vec3D up = { 0.0f, 1.0f, 0.0f };

	Mat4X4 view = MathMat4X4ViewAt(&lightPos, &targetPos, &up);

	// Transform bounding sphere to light space.
	Vec4D targetPos4 = { targetPos.X, targetPos.Y, targetPos.Z, 1.0f };
	Vec4D sphereCenterLS = MathMat4X4MultVec4DByMat4X4(&targetPos4 , &view);

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.X - sceneBounds.Radius;
	float b = sphereCenterLS.Y - sceneBounds.Radius;
	float n = sphereCenterLS.Z - sceneBounds.Radius;
	float r = sphereCenterLS.X + sceneBounds.Radius;
	float t = sphereCenterLS.Y + sceneBounds.Radius;
	float f = sphereCenterLS.Z + sceneBounds.Radius;
	Mat4X4 proj = MathMat4X4OrthographicOffCenter(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	Mat4X4 T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	Mat4X4 shadowTransform = view * proj * T;

	m_PerFrameData.shadowTransform = shadowTransform;
	m_PerFrameData.cameraPosW = lightPos;
	m_PerFrameData.proj = proj;
	m_PerFrameData.view = view;

	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());
}