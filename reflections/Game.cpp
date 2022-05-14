#include "Game.h"
#include "Utils.h"
#include "Math.h"
#include "Camera.h"
#include "MeshGenerator.h"

#if WITH_IMGUI
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#endif

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

void Game::CreatePixelShader(const char* filepath, ID3D11Device* device, ID3D11PixelShader** ps)
{
	auto bytes = UtilsReadData(UtilsFormatStr("%s/%s", SHADERS_ROOT, filepath).c_str());

	if (FAILED(device->CreatePixelShader(&bytes[0], bytes.size(), NULL, ps)))
	{
		UTILS_FATAL_ERROR("Failed to create pixel shader from %s", filepath);
	}
}

const Actor* Game::FindActorByName(const std::string& name) const
{
	auto it = std::find_if(std::begin(m_Actors), std::end(m_Actors), [&name](const Actor& actor)
		{
			return actor.GetName() == name;
		});
	return it == std::end(m_Actors) ? nullptr : &*it;
}

#if WITH_IMGUI
void Game::UpdateImgui()
{
	// Any application code here
	ImGui::Checkbox("Rotate dir light", &m_ImguiState.RotateDirLight);
	ImGui::Checkbox("Animate first point light", &m_ImguiState.AnimatePointLight);
	ImGui::Checkbox("Capture spotlight", &m_ImguiState.ToggleSpotlight);
	static float zNear = 0.5f;
	ImGui::SliderFloat("Z near", &zNear, 0.1f, 10.0f, "%f", 1.0f);
	static float zFar = 200.0f;
	ImGui::SliderFloat("Z far", &zFar, 10.0f, 1000.0f, "%f", 1.0f);
	m_Camera.SetZFar(zFar);
	m_Camera.SetZNear(zNear);
	ImGui::SliderFloat("Bounding sphere radius", &m_sceneBounds.Radius, 10.0f, 500.0f, "%f", 1.0f);
}
#endif

std::vector<uint8_t> Game::CreateVertexShader(const char* filepath, ID3D11Device* device, ID3D11VertexShader** vs)
{
	auto bytes = UtilsReadData(UtilsFormatStr("%s/%s", SHADERS_ROOT, filepath).c_str());

	if (FAILED(device->CreateVertexShader(&bytes[0], bytes.size(), NULL, vs)))
	{
		UTILS_FATAL_ERROR("Failed to create vertex shader from %s", filepath);
	}
	return bytes;
}

void Game::CreateDefaultSampler()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.MipLODBias = 0.0f;
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
		{-4.0f, 0.4f, -4.0f},
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
	dirLight.Ambient = ColorFromRGBA(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight.Diffuse = ColorFromRGBA(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.Specular = ColorFromRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	dirLight.Direction = MathVec3DFromXYZ(1.0f, 1.0f, 1.0f);
	m_PerSceneData.dirLight = dirLight;

	SpotLight spotLight = {};
	spotLight.Position = m_Camera.GetPos();
	spotLight.Direction = m_Camera.GetAt();
	spotLight.Ambient = ColorFromRGBA(0.1f, 0.1f, 0.1f, 1.0f);
	spotLight.Diffuse = ColorFromRGBA(1.0f, 0.0f, 0.0f, 1.0f);
	spotLight.Specular = ColorFromRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	spotLight.Att = MathVec3DFromXYZ(1.0f, 0.09f, 0.032f);
	spotLight.Range = 5.0f;
	spotLight.Spot = 32.0f;
	m_PerSceneData.spotLights[0] = spotLight;
}

Game::Game():
	m_Camera{ {0.0f, 0.0f, -5.0f} }
{
	m_DR = std::make_unique<DeviceResources>();
	m_sceneBounds.Center = { 0.0f, 0.0f, 0.0f };
	m_sceneBounds.Radius = 100.0f;
}

Game::~Game()
{
#if WITH_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
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

#if WITH_IMGUI
	// update directional light
	static float elapsedTime = 0.0f;
	elapsedTime += (float)m_Timer.DeltaMillis / (1000.0f * 2.0f);
	if (m_ImguiState.RotateDirLight)
	{
		m_PerSceneData.dirLight.Direction.X = sinf(elapsedTime);
		m_PerSceneData.dirLight.Direction.Y = 1.0f;
		m_PerSceneData.dirLight.Direction.Z = cosf(elapsedTime);
		MathVec3DNormalize(&m_PerSceneData.dirLight.Direction);

		const Vec3D at = m_Camera.GetAt();
		const Vec3D pos = m_Camera.GetPos();

		m_PerSceneData.spotLights[0].Position = pos;
		m_PerSceneData.spotLights[0].Direction = at;
	}

	if (m_ImguiState.AnimatePointLight)
	{
		static const float radius = 3.0f;
		m_PerSceneData.pointLights[0].Position.X = sinf(elapsedTime) * radius;
		m_PerSceneData.pointLights[0].Position.Z = cosf(elapsedTime) * radius;
		const float color = cosf(elapsedTime) * cosf(elapsedTime);

		m_PerSceneData.pointLights[0].Diffuse.R = color;
		m_PerSceneData.pointLights[0].Diffuse.G = 1.0f - color;
	}

	if (m_ImguiState.ToggleSpotlight)
	{
		m_PerSceneData.spotLights[0].Direction = m_Camera.GetAt();
		m_PerSceneData.spotLights[0].Position = m_Camera.GetPos();
	}

	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerSceneConstants), &m_PerSceneData, m_PerSceneCB.Get());
#endif
}

void Game::Render()
{
#if WITH_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

#if WITH_IMGUI
	UpdateImgui();
#endif

	m_Renderer.Clear();
	m_Renderer.SetDepthStencilState(nullptr);
	m_Renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
	m_Renderer.SetRasterizerState(m_DR->GetRasterizerState());
	m_Renderer.SetSamplerState(m_DefaultSampler.Get(), 0);

	m_DR->PIXBeginEvent(L"Shadow pass");
	m_Renderer.SetInputLayout(m_InputLayout.GetDefaultLayout());

	m_ShadowMap.Bind(m_DR->GetDeviceContext());
	{
		m_Renderer.BindVertexShader(m_VS.Get());
		m_Renderer.BindPixelShader(nullptr);
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerSceneCB.Get(), 2);

		for (size_t i = 0; i < m_Actors.size(); ++i)
		{
			const Actor& actor = m_Actors[i];
			if (actor.IsVisible())
			{
				m_PerObjectData.worldInvTranspose = MathMat4X4Inverse(actor.GetWorld());
				m_PerObjectData.world = actor.GetWorld();
				m_PerObjectData.material = actor.GetMaterial();
				GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
					sizeof(PerObjectConstants),
					&m_PerObjectData,
					m_PerObjectCB.Get());
				m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
				m_Renderer.SetIndexBuffer(actor.GetIndexBuffer(), 0);
				m_Renderer.SetVertexBuffer(actor.GetVertexBuffer(), m_InputLayout.GetVertexSize(InputLayout::VertexType::Default), 0);

				m_Renderer.DrawIndexed(actor.GetNumIndices(), 0, 0);
			}
		}
	}
	m_ShadowMap.Unbind(m_DR->GetDeviceContext());
	m_DR->PIXEndEvent();
	m_DR->PIXBeginEvent(L"Color pass");
	// reset view proj matrix back to camera
	{
		m_PerFrameData.view = m_Camera.GetViewMat();
		m_PerFrameData.proj = m_Camera.GetProjMat();
		m_PerFrameData.cameraPosW = m_Camera.GetPos();
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());
	}

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
		if (actor.IsVisible())
		{
			m_Renderer.BindShaderResources(BindTargets::PixelShader, actor.GetShaderResources(), ACTOR_NUM_TEXTURES);
			m_PerObjectData.world = actor.GetWorld();
			m_PerObjectData.material = actor.GetMaterial();
			m_PerObjectData.worldInvTranspose = MathMat4X4Inverse(actor.GetWorld());
			GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
				sizeof(PerObjectConstants),
				&m_PerObjectData,
				m_PerObjectCB.Get());
			m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
			m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);

			m_Renderer.SetIndexBuffer(actor.GetIndexBuffer(), 0);
			m_Renderer.SetVertexBuffer(actor.GetVertexBuffer(), m_InputLayout.GetVertexSize(InputLayout::VertexType::Default), 0);

			m_Renderer.DrawIndexed(actor.GetNumIndices(), 0, 0);
		}
	}
	m_DR->PIXEndEvent();
	m_DR->PIXBeginEvent(L"Draw lights");
	// Light properties
	//for (uint32_t i = 0; i < _countof(m_PerSceneData.pointLights); ++i)
	{
		const Vec3D scale = { 0.2f, 0.2f, 0.2f };
		Mat4X4 world = MathMat4X4ScaleFromVec3D(&scale);
		Vec3D dirLightPos = m_PerSceneData.dirLight.Direction;
		dirLightPos = MathVec3DModulateByScalar(&dirLightPos, m_sceneBounds.Radius);
		Mat4X4 translate = MathMat4X4TranslateFromVec3D(&dirLightPos);
		m_PerObjectData.world = MathMat4X4MultMat4X4ByMat4X4(&world, &translate);
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
			sizeof(PerObjectConstants),
			&m_PerObjectData,
			m_PerObjectCB.Get());

		m_Renderer.BindPixelShader(m_LightPS.Get());
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);
		const auto sphere = FindActorByName("Sphere");
		m_Renderer.SetIndexBuffer(sphere->GetIndexBuffer(), 0);
		m_Renderer.SetVertexBuffer(sphere->GetVertexBuffer(), m_InputLayout.GetVertexSize(InputLayout::VertexType::Default), 0);

		m_Renderer.DrawIndexed(sphere->GetNumIndices(), 0, 0);
	}
	m_DR->PIXEndEvent();
	// TODO: Need to have a reflection mechanism to query amount of SRV that is possible to bind to PS
	// After this this clear code could be placed to Renderer::Clear
	ID3D11ShaderResourceView* nullSRVs[5] = { nullptr, nullptr, nullptr, nullptr, nullptr, };
	m_Renderer.BindShaderResources(BindTargets::PixelShader, nullSRVs, 5);

	// draw sky
	m_DR->PIXBeginEvent(L"Draw sky");
	m_Renderer.SetRasterizerState(m_CubeMap.GetRasterizerState());
	m_Renderer.SetDepthStencilState(m_CubeMap.GetDepthStencilState());
	m_Renderer.SetInputLayout(m_InputLayout.GetSkyLayout());
	m_Renderer.BindVertexShader(m_SkyVS.Get());
	m_Renderer.BindPixelShader(m_SkyPS.Get());
	m_Renderer.BindShaderResource(BindTargets::PixelShader, m_CubeMap.GetCubeMap(), 0);
	m_Renderer.SetSamplerState(m_CubeMap.GetCubeMapSampler(), 0);
	m_Renderer.SetVertexBuffer(m_CubeMap.GetVertexBuffer(), m_InputLayout.GetVertexSize(InputLayout::VertexType::Sky), 0);
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
	m_Renderer.SetIndexBuffer(m_CubeMap.GetIndexBuffer(), 0);
	m_Renderer.DrawIndexed(m_CubeMap.GetNumIndices(), 0, 0);

	m_DR->PIXEndEvent();

#if WITH_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

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
	const std::string names[] = {
		"Cube",
		"Sphere"
	};

	const bool visibility[] = {
		true,
		false
	};

	const std::string models[] = {
		//UtilsFormatStr("%s/meshes/podium.obj", ASSETS_ROOT),
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
		actor.SetName(names[i]);
		actor.SetIsVisible(visibility[i]);

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
		
		plane.LoadTexture(UtilsFormatStr("%s/textures/marble_diffuse.jpg", ASSETS_ROOT).c_str(), TextureType::Diffuse, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/marble_normal.png", ASSETS_ROOT).c_str(), TextureType::Normal, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/marble_gloss.jpg", ASSETS_ROOT).c_str(), TextureType::Gloss, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/marble_reflection.jpg", ASSETS_ROOT).c_str(), TextureType::Specular, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.SetMaterial(&material);
		plane.SetIsVisible(true);
		plane.SetName("Plane");
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
	m_CubeMap.LoadCubeMap(m_DR->GetDevice(), {
		UtilsFormatStr("%s/textures/right.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/left.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/top.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/bottom.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/front.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/back.jpg", ASSETS_ROOT).c_str(),
		});

	// init actors
	ID3D11Device* device = m_DR->GetDevice();
	CreateActors();
	m_CubeMap.CreateCube(*FindActorByName("Cube"), device);
	InitPerSceneConstants();

	CreatePixelShader("PixelShader.cso", (ID3D11Device*)device, m_PS.ReleaseAndGetAddressOf());
	CreatePixelShader("PhongPS.cso", (ID3D11Device*)device, m_PhongPS.ReleaseAndGetAddressOf());
	CreatePixelShader("LightPS.cso", (ID3D11Device*)device, m_LightPS.ReleaseAndGetAddressOf());
	CreatePixelShader("SkyPS.cso", device, m_SkyPS.ReleaseAndGetAddressOf());
	auto bytes = CreateVertexShader("VertexShader.cso", (ID3D11Device*)device, m_VS.ReleaseAndGetAddressOf());
	m_InputLayout.CreateDefaultLayout(device, &bytes[0], bytes.size());
	bytes = CreateVertexShader("SkyVS.cso", device, m_SkyVS.ReleaseAndGetAddressOf());
	m_InputLayout.CreateSkyLayout(device, &bytes[0], bytes.size());

	GameCreateConstantBuffer(m_DR->GetDevice(), sizeof(PerSceneConstants), &m_PerSceneCB);
	GameCreateConstantBuffer(m_DR->GetDevice(), sizeof(PerObjectConstants), &m_PerObjectCB);
	GameCreateConstantBuffer(m_DR->GetDevice(), sizeof(PerFrameConstants), &m_PerFrameCB);
	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerSceneConstants), &m_PerSceneData, m_PerSceneCB.Get());

	CreateDefaultSampler();

	m_Renderer.SetDeviceResources(m_DR.get());

#if WITH_IMGUI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable some options
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(m_DR->GetDevice(), m_DR->GetDeviceContext());
#endif
}

void Game::GetDefaultSize(uint32_t* width, uint32_t* height)
{
	*width = DEFAULT_WIN_WIDTH;
	*height = DEFAULT_WIN_HEIGHT;
}

void Game::BuildShadowTransform()
{
	// Only the first "main" light casts a shadow.
	Vec3D lightDir = m_PerSceneData.dirLight.Direction;
	Vec3D lightPos = MathVec3DModulateByScalar(&lightDir, m_sceneBounds.Radius);
	Vec3D targetPos = { 0.0f, 0.0f, 0.0f };
	Vec3D up = { 0.0f, 1.0f, 0.0f };

	Mat4X4 view = MathMat4X4ViewAt(&lightPos, &targetPos, &up);

	// Transform bounding sphere to light space.
	Vec4D targetPos4 = { targetPos.X, targetPos.Y, targetPos.Z, 1.0f };
	Vec4D sphereCenterLS = MathMat4X4MultVec4DByMat4X4(&targetPos4 , &view);

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.X - m_sceneBounds.Radius;
	float b = sphereCenterLS.Y - m_sceneBounds.Radius;
	float n = sphereCenterLS.Z - m_sceneBounds.Radius;
	float r = sphereCenterLS.X + m_sceneBounds.Radius;
	float t = sphereCenterLS.Y + m_sceneBounds.Radius;
	float f = sphereCenterLS.Z + m_sceneBounds.Radius;
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