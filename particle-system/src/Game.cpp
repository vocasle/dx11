#include "Camera.h"
#include "Game.h"
#include "MeshGenerator.h"
#include "Mouse.h"
#include "NE_Math.h"
#include "Utils.h"
#include "particle-system_config.h"

#if WITH_IMGUI
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#endif

namespace Globals
{
struct ParticleSystemOptions {
	bool isEnabled = true;
	Vec3D origin = {};
	float lifespan = 0;
	int maxParticles = 0;
	ParticleSystemOptions(const ParticleSystem &ps)
		: origin(ps.GetOrigin())
		, lifespan(ps.GetLifespan())
		, maxParticles(ps.GetMaxParticles())
	{
	}
};

std::unique_ptr<ParticleSystemOptions> FireOptions;
std::unique_ptr<ParticleSystemOptions> RainOptions;
};

static void GameUpdateConstantBuffer(ID3D11DeviceContext *context,
				     size_t bufferSize, void *data,
				     ID3D11Buffer *dest)
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	if (FAILED(context->Map((ID3D11Resource *)dest, 0,
				D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
		UtilsFatalError("ERROR: Failed to map constant buffer\n");
	}
	memcpy(mapped.pData, data, bufferSize);
	context->Unmap((ID3D11Resource *)dest, 0);
}

static void GameCreateConstantBuffer(ID3D11Device *device, size_t byteWidth,
				     ID3D11Buffer **pDest)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = byteWidth;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&bufferDesc, NULL, pDest))) {
		UtilsFatalError(
			"ERROR: Failed to create per frame constants cbuffer\n");
	}
}

void Game::CreatePixelShader(const char *filepath, ID3D11Device *device,
			     ID3D11PixelShader **ps)
{
	auto bytes = UtilsReadData(
		UtilsFormatStr("%s/%s", SHADERS_ROOT, filepath).c_str());

	if (FAILED(device->CreatePixelShader(&bytes[0], bytes.size(), NULL,
					     ps))) {
		UTILS_FATAL_ERROR("Failed to create pixel shader from %s",
				  filepath);
	}
}

Actor *Game::FindActorByName(const std::string &name)
{
	auto it = std::find_if(std::begin(m_Actors), std::end(m_Actors),
			       [&name](const Actor &actor) {
				       return actor.GetName() == name;
			       });
	return it == std::end(m_Actors) ? nullptr : &*it;
}

// TODO: Update this to get list of actors to draw
void Game::DrawScene()
{
	m_Renderer.BindPixelShader(m_shaderManager.GetPixelShader("PhongPS"));
	m_Renderer.BindVertexShader(
		m_shaderManager.GetVertexShader("VertexShaderVS"));
	m_Renderer.SetInputLayout(m_shaderManager.GetInputLayout());

	m_Renderer.BindShaderResource(BindTargets::PixelShader,
				      m_ShadowMap.GetDepthMapSRV(), 4);
	m_Renderer.SetSamplerState(m_ShadowMap.GetShadowSampler(), 1);

	m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
				      m_PerFrameCB.Get(), 1);
	m_Renderer.BindConstantBuffer(BindTargets::PixelShader,
				      m_PerFrameCB.Get(), 1);

	m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
				      m_PerSceneCB.Get(), 2);
	m_Renderer.BindConstantBuffer(BindTargets::PixelShader,
				      m_PerSceneCB.Get(), 2);

	for (size_t i = 0; i < m_Actors.size(); ++i) {
		const Actor &actor = m_Actors[i];
		if (actor.IsVisible() && actor.GetName() == "Plane") {
			m_Renderer.BindShaderResources(
				BindTargets::PixelShader,
				actor.GetShaderResources(), ACTOR_NUM_TEXTURES);
			m_PerObjectData.world = actor.GetWorld();
			m_PerObjectData.material = actor.GetMaterial();
			m_PerObjectData.worldInvTranspose =
				MathMat4X4Inverse(actor.GetWorld());
			GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
						 sizeof(PerObjectConstants),
						 &m_PerObjectData,
						 m_PerObjectCB.Get());
			m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
						      m_PerObjectCB.Get(), 0);
			m_Renderer.BindConstantBuffer(BindTargets::PixelShader,
						      m_PerObjectCB.Get(), 0);

			m_Renderer.SetIndexBuffer(actor.GetIndexBuffer(), 0);
			m_Renderer.SetVertexBuffer(actor.GetVertexBuffer(),
						   m_shaderManager.GetStrides(),
						   0);

			m_Renderer.DrawIndexed(actor.GetNumIndices(), 0, 0);
		}
	}
}

void Game::DrawSky()
{
	m_DR->PIXBeginEvent(L"Draw sky");
	m_Renderer.SetRasterizerState(m_CubeMap.GetRasterizerState());
	m_Renderer.SetDepthStencilState(m_CubeMap.GetDepthStencilState());
	m_Renderer.BindVertexShader(m_shaderManager.GetVertexShader("SkyVS"));
	m_Renderer.BindPixelShader(m_shaderManager.GetPixelShader("SkyPS"));
	m_Renderer.SetInputLayout(m_shaderManager.GetInputLayout());
	m_Renderer.BindShaderResource(BindTargets::PixelShader,
				      m_CubeMap.GetCubeMap(), 0);
	m_Renderer.SetSamplerState(m_CubeMap.GetCubeMapSampler(), 0);
	m_Renderer.SetVertexBuffer(m_CubeMap.GetVertexBuffer(),
				   m_shaderManager.GetStrides(), 0);
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
				      m_PerObjectCB.Get(), 0);
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
				      m_PerFrameCB.Get(), 1);
	m_Renderer.SetIndexBuffer(m_CubeMap.GetIndexBuffer(), 0);
	m_Renderer.DrawIndexed(m_CubeMap.GetNumIndices(), 0, 0);
	m_DR->PIXEndEvent();
}

#if WITH_IMGUI
void Game::UpdateImgui()
{
	// Any application code here
	static float zNear = 0.5f;
	ImGui::SliderFloat("Z near", &zNear, 0.1f, 10.0f, "%f", 1.0f);
	static float zFar = 200.0f;
	ImGui::SliderFloat("Z far", &zFar, 10.0f, 1000.0f, "%f", 1.0f);
	if (m_Camera.GetZFar() != zFar)
		m_Camera.SetZFar(zFar);
	if (m_Camera.GetZNear() != zNear)
		m_Camera.SetZNear(zNear);

	if (ImGui::CollapsingHeader("Fire")) {
		ImGui::Checkbox("Enable fire",
				&Globals::FireOptions->isEnabled);
		ImGui::InputFloat("Fire lifespan",
				  &Globals::FireOptions->lifespan);
		if (m_fire.GetLifespan() != Globals::FireOptions->lifespan)
			m_fire.SetLifespan(Globals::FireOptions->lifespan);
	}
	if (ImGui::CollapsingHeader("Rain")) {
		ImGui::Checkbox("Enable rain",
				&Globals::RainOptions->isEnabled);
		ImGui::InputFloat("Rain lifespan",
				  &Globals::RainOptions->lifespan);
		if (m_rain.GetLifespan() != Globals::RainOptions->lifespan)
			m_rain.SetLifespan(Globals::RainOptions->lifespan);

		ImGui::InputInt("Rain num particles",
				&Globals::RainOptions->maxParticles);
		if (m_rain.GetMaxParticles() <
		    Globals::RainOptions->maxParticles)
			m_rain.SetMaxParticles(
				Globals::RainOptions->maxParticles);
	}
}
#endif

std::vector<uint8_t> Game::CreateVertexShader(const char *filepath,
					      ID3D11Device *device,
					      ID3D11VertexShader **vs)
{
	auto bytes = UtilsReadData(
		UtilsFormatStr("%s/%s", SHADERS_ROOT, filepath).c_str());

	if (FAILED(device->CreateVertexShader(&bytes[0], bytes.size(), NULL,
					      vs))) {
		UTILS_FATAL_ERROR("Failed to create vertex shader from %s",
				  filepath);
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

	if (FAILED(m_DR->GetDevice()->CreateSamplerState(
		    &samplerDesc, m_DefaultSampler.ReleaseAndGetAddressOf()))) {
		UtilsDebugPrint(
			"ERROR: Failed to create default sampler state\n");
		ExitProcess(EXIT_FAILURE);
	}
}

void Game::InitPerSceneConstants()
{
	struct PointLight pl = {};
	const Vec3D positions[] = {
		{ -4.0f, 0.4f, -4.0f },
		{ -4.0f, 1.5f, 4.0f },
		{ 4.0f, 1.5f, 4.0f },
		{ 4.0f, 1.5f, -4.0f },
	};

	for (uint32_t i = 0; i < 4; ++i) {
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
	dirLight.Position = MathVec3DFromXYZ(10.0f, 10.0f, 10.0f);
	dirLight.Radius = MathVec3DLength(dirLight.Position);
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

Game::Game()
	: m_Camera{ { 0.0f, 0.0f, -5.0f } }
	, m_fire{ "Fire", { 0.0f, 0.0f, 0.0f }, m_Camera }
	, m_rain{ "Rain", { 0, 100, 0 }, m_Camera }
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
	ID3D11DeviceContext *ctx = m_DR->GetDeviceContext();
	ID3D11RenderTargetView *rtv = m_DR->GetRenderTargetView();
	ID3D11DepthStencilView *dsv = m_DR->GetDepthStencilView();

	static const float CLEAR_COLOR[4] = { 0.392156899f, 0.584313750f,
					      0.929411829f, 1.000000000f };

	ctx->Flush();

	ctx->ClearRenderTargetView(rtv, CLEAR_COLOR);
	ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				   1.0f, 0);
	ctx->OMSetRenderTargets(1, &rtv, dsv);
	ctx->RSSetViewports(1, &m_DR->GetViewport());
}

void Game::Update()
{
	m_Camera.ProcessKeyboard(m_Timer.DeltaMillis);
	m_Camera.ProcessMouse(m_Timer.DeltaMillis);

	m_PerFrameData.view = m_Camera.GetViewMat();
	m_PerFrameData.proj = m_Camera.GetProjMat();
	m_PerFrameData.cameraPosW = m_Camera.GetPos();

	GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
				 sizeof(PerFrameConstants), &m_PerFrameData,
				 m_PerFrameCB.Get());

	m_fire.Tick(static_cast<float>(m_Timer.DeltaMillis / 1000));
	m_fire.UpdateVertexBuffer(m_DR->GetDeviceContext());

	m_rain.Tick(static_cast<float>(m_Timer.DeltaMillis / 1000));
	m_rain.UpdateVertexBuffer(m_DR->GetDeviceContext());

	static float elapsedTime = 0.0f;
	const float deltaSeconds =
		static_cast<float>(m_Timer.DeltaMillis / 1000.0);
	elapsedTime += deltaSeconds;

	if (elapsedTime >= 1.0f) {
		SetWindowText(
			m_DR->GetWindow(),
			UtilsFormatStr(
				"Particles -- FPS: %d, frame: %f s, alive particles: %d",
				static_cast<int>(elapsedTime / deltaSeconds),
				deltaSeconds, m_fire.GetNumAliveParticles())
				.c_str());
		elapsedTime = 0.0f;
	}

#if WITH_IMGUI
	// update directional light
	elapsedTime += (float)m_Timer.DeltaMillis / (1000.0f * 2.0f);
	if (m_ImguiState.RotateDirLight) {
		m_PerSceneData.dirLight.Position.X =
			m_PerSceneData.dirLight.Radius * sinf(elapsedTime);
		m_PerSceneData.dirLight.Position.Z =
			m_PerSceneData.dirLight.Radius * cosf(elapsedTime);

		const Vec3D at = m_Camera.GetAt();
		const Vec3D pos = m_Camera.GetPos();

		m_PerSceneData.spotLights[0].Position = pos;
		m_PerSceneData.spotLights[0].Direction = at;
	}

	if (m_ImguiState.AnimatePointLight) {
		static const float radius = 3.0f;
		m_PerSceneData.pointLights[0].Position.X =
			sinf(elapsedTime) * radius;
		m_PerSceneData.pointLights[0].Position.Z =
			cosf(elapsedTime) * radius;
		const float color = cosf(elapsedTime) * cosf(elapsedTime);

		m_PerSceneData.pointLights[0].Diffuse.R = color;
		m_PerSceneData.pointLights[0].Diffuse.G = 1.0f - color;
	}

	if (m_ImguiState.ToggleSpotlight) {
		m_PerSceneData.spotLights[0].Direction = m_Camera.GetAt();
		m_PerSceneData.spotLights[0].Position = m_Camera.GetPos();
	}

	GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
				 sizeof(PerSceneConstants), &m_PerSceneData,
				 m_PerSceneCB.Get());
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

	m_Renderer.SetBlendState(nullptr);
	m_Renderer.SetDepthStencilState(nullptr);
	m_Renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
	m_Renderer.SetRasterizerState(m_DR->GetRasterizerState());
	m_Renderer.SetViewport(m_DR->GetViewport());
	m_Renderer.SetSamplerState(m_DefaultSampler.Get(), 0);
	m_Renderer.SetRenderTargets(m_DR->GetRenderTargetView(),
				    m_DR->GetDepthStencilView());
	m_Renderer.Clear(nullptr);

	m_DR->PIXBeginEvent(L"Color pass");
	// reset view proj matrix back to camera
	{
		m_PerFrameData.view = m_Camera.GetViewMat();
		m_PerFrameData.proj = m_Camera.GetProjMat();
		m_PerFrameData.cameraPosW = m_Camera.GetPos();
		m_Renderer.BindShaderResource(BindTargets::PixelShader,
					      m_dynamicCubeMap.GetSRV(), 6);
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
					 sizeof(PerFrameConstants),
					 &m_PerFrameData, m_PerFrameCB.Get());
		DrawScene();
	}
	m_DR->PIXEndEvent();
	m_DR->PIXBeginEvent(L"Draw lights");
	// Light properties
	// for (uint32_t i = 0; i < _countof(m_PerSceneData.pointLights); ++i)
	{
		m_PerObjectData.world = MathMat4X4TranslateFromVec3D(
			&m_PerSceneData.dirLight.Position);
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
					 sizeof(PerObjectConstants),
					 &m_PerObjectData, m_PerObjectCB.Get());

		m_Renderer.BindPixelShader(
			m_shaderManager.GetPixelShader("LightPS"));
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
					      m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader,
					      m_PerObjectCB.Get(), 0);
		const auto sphere = FindActorByName("Sphere");
		m_Renderer.SetIndexBuffer(sphere->GetIndexBuffer(), 0);
		m_Renderer.SetVertexBuffer(sphere->GetVertexBuffer(),
					   m_shaderManager.GetStrides(), 0);

		m_Renderer.DrawIndexed(sphere->GetNumIndices(), 0, 0);
	}
	m_DR->PIXEndEvent();
	// TODO: Need to have a reflection mechanism to query amount of SRV that is
	// possible to bind to PS After this this clear code could be placed to
	// Renderer::Clear
	ID3D11ShaderResourceView *nullSRVs[5] = {
		nullptr, nullptr, nullptr, nullptr, nullptr,
	};
	m_Renderer.BindShaderResources(BindTargets::PixelShader, nullSRVs, 5);

	if (Globals::FireOptions->isEnabled) {
		m_DR->PIXBeginEvent(L"Draw fire");
		{
			m_Renderer.SetBlendState(m_fire.GetBlendState());
			m_Renderer.SetDepthStencilState(
				m_fire.GetDepthStencilState());
			m_Renderer.BindVertexShader(
				m_shaderManager.GetVertexShader("ParticleVS"));
			m_Renderer.SetVertexBuffer(m_fire.GetVertexBuffer(),
						   m_shaderManager.GetStrides(),
						   0);
			m_Renderer.SetIndexBuffer(m_fire.GetIndexBuffer(), 0);
			m_Renderer.BindPixelShader(
				m_shaderManager.GetPixelShader("ParticlePS"));
			m_Renderer.SetInputLayout(
				m_shaderManager.GetInputLayout());
			m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
						      m_PerFrameCB.Get(), 0);
			m_Renderer.BindShaderResource(BindTargets::PixelShader,
						      m_fire.GetSRV(), 0);
			m_Renderer.SetSamplerState(m_fire.GetSamplerState(), 0);
			m_Renderer.DrawIndexed(m_fire.GetNumIndices(), 0, 0);
		}
		m_DR->PIXEndEvent();
	}

	if (Globals::RainOptions->isEnabled) {
		m_DR->PIXBeginEvent(L"Draw rain");
		{
			m_Renderer.SetBlendState(m_rain.GetBlendState());
			m_Renderer.SetDepthStencilState(
				m_rain.GetDepthStencilState());
			m_Renderer.BindVertexShader(
				m_shaderManager.GetVertexShader("ParticleVS"));
			m_Renderer.SetVertexBuffer(m_rain.GetVertexBuffer(),
						   m_shaderManager.GetStrides(),
						   0);
			m_Renderer.SetIndexBuffer(m_rain.GetIndexBuffer(), 0);
			m_Renderer.BindPixelShader(
				m_shaderManager.GetPixelShader("ParticlePS"));
			m_Renderer.SetInputLayout(
				m_shaderManager.GetInputLayout());
			m_Renderer.BindConstantBuffer(BindTargets::VertexShader,
						      m_PerFrameCB.Get(), 0);
			m_Renderer.BindShaderResource(BindTargets::PixelShader,
						      m_rain.GetSRV(), 0);
			m_Renderer.SetSamplerState(m_rain.GetSamplerState(), 0);
			m_Renderer.DrawIndexed(m_rain.GetNumIndices(), 0, 0);
		}
		m_DR->PIXEndEvent();
	}

	// draw sky
	DrawSky();

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
	// load sphere
	{
		Actor actor = Actor(MGCreateSphere(1.0f, 20, 20).get());
		actor.Translate({ 0.0f, 1.0f, 0.0f });
		actor.CreateIndexBuffer(m_DR->GetDevice());
		actor.CreateVertexBuffer(m_DR->GetDevice());
		actor.LoadTexture(
			UtilsFormatStr("%s/textures/metall_sheet_diffuse.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Diffuse, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		actor.LoadTexture(
			UtilsFormatStr("%s/textures/metall_sheet_specular.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Specular, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		actor.LoadTexture(
			UtilsFormatStr("%s/textures/metall_sheet_gloss.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Gloss, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		actor.LoadTexture(
			UtilsFormatStr("%s/textures/metall_sheet_normal.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Normal, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		Material material({ 0.23125f, 0.23125f, 0.23125f, 1.0f },
				  { 0.2775f, 0.2775f, 0.2775f, 1.0f },
				  { 0.773911f, 0.773911f, 0.773911f, 89.6f },
				  { 0.5f, 0.5f, 0.5f, 1.0f });
		actor.SetMaterial(&material);
		actor.SetName("Sphere");
		actor.SetIsVisible(true);

		m_Actors.emplace_back(actor);
	}

	// load cube
	{
		Actor actor = Actor();
		actor.LoadModel(
			UtilsFormatStr("%s/meshes/cube.obj", ASSETS_ROOT)
				.c_str());
		actor.CreateIndexBuffer(m_DR->GetDevice());
		actor.CreateVertexBuffer(m_DR->GetDevice());
		actor.SetName("Cube");
		actor.SetIsVisible(false);

		m_Actors.emplace_back(actor);
	}

	// load plane
	{
		const Vec3D origin = { 0.0f, 0.0f, 0.0f };
		std::unique_ptr<Mesh> mesh =
			MGGeneratePlane(&origin, 10.0f, 10.0f);
		Actor plane = Actor(mesh.get());
		plane.CreateIndexBuffer(m_DR->GetDevice());
		plane.CreateVertexBuffer(m_DR->GetDevice());
		const Vec3D offset = { 0.0f, -1.0f, 0.0f };
		plane.Translate(offset);

		plane.LoadTexture(
			UtilsFormatStr(
				"%s/textures/wood_floor_diffuseOriginal.jpg",
				ASSETS_ROOT)
				.c_str(),
			TextureType::Diffuse, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		plane.LoadTexture(
			UtilsFormatStr("%s/textures/wood_floor_normal.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Normal, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		plane.LoadTexture(
			UtilsFormatStr("%s/textures/wood_floor_smoothness.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Gloss, m_DR->GetDevice(),
			m_DR->GetDeviceContext());
		plane.LoadTexture(
			UtilsFormatStr("%s/textures/wood_floor_metallic.jpg",
				       ASSETS_ROOT)
				.c_str(),
			TextureType::Specular, m_DR->GetDevice(),
			m_DR->GetDeviceContext());

		const Material material = {
			{ 0.24725f, 0.1995f, 0.0745f, 1.0f },
			{ 0.75164f, 0.60648f, 0.22648f, 1.0f },
			{ 0.628281f, 0.555802f, 0.366065f, 32.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f }
		};
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
	Mouse::Get().SetWindowDimensions(m_DR->GetBackBufferWidth(),
					 m_DR->GetBackBufferHeight());
	ID3D11Device *device = m_DR->GetDevice();

	m_ShadowMap.InitResources(device, 2048, 2048);
	m_Camera.SetViewDimensions(m_DR->GetBackBufferWidth(),
				   m_DR->GetBackBufferHeight());
	m_CubeMap.LoadCubeMap(
		device,
		{
			UtilsFormatStr("%s/textures/right.jpg", ASSETS_ROOT)
				.c_str(),
			UtilsFormatStr("%s/textures/left.jpg", ASSETS_ROOT)
				.c_str(),
			UtilsFormatStr("%s/textures/top.jpg", ASSETS_ROOT)
				.c_str(),
			UtilsFormatStr("%s/textures/bottom.jpg", ASSETS_ROOT)
				.c_str(),
			UtilsFormatStr("%s/textures/front.jpg", ASSETS_ROOT)
				.c_str(),
			UtilsFormatStr("%s/textures/back.jpg", ASSETS_ROOT)
				.c_str(),
		});

	m_dynamicCubeMap.Init(device);
	m_dynamicCubeMap.BuildCubeFaceCamera({ 0.0f, 0.0f, 0.0f });
	// init actors
	CreateActors();
	m_CubeMap.CreateCube(*FindActorByName("Cube"), device);
	InitPerSceneConstants();

	m_shaderManager.Initialize(device, SHADERS_ROOT,
				   UtilsFormatStr("%s/shader", SRC_ROOT));

	GameCreateConstantBuffer(device, sizeof(PerSceneConstants),
				 &m_PerSceneCB);
	GameCreateConstantBuffer(device, sizeof(PerObjectConstants),
				 &m_PerObjectCB);
	GameCreateConstantBuffer(device, sizeof(PerFrameConstants),
				 &m_PerFrameCB);
	GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
				 sizeof(PerSceneConstants), &m_PerSceneData,
				 m_PerSceneCB.Get());

	CreateDefaultSampler();

	m_Renderer.SetDeviceResources(m_DR.get());

	m_fire.Init(
		device, m_DR->GetDeviceContext(),
		UtilsFormatStr("%s/textures/flare0.png", ASSETS_ROOT).c_str());

	m_rain.Init(
		device, m_DR->GetDeviceContext(),
		UtilsFormatStr("%s/textures/trace_01.png", ASSETS_ROOT).c_str());

	{ // Init globals
		Globals::FireOptions =
			std::make_unique<Globals::ParticleSystemOptions>(
				m_fire);
		Globals::RainOptions =
			std::make_unique<Globals::ParticleSystemOptions>(
				m_rain);
	}

#if WITH_IMGUI
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |=
		ImGuiConfigFlags_NavEnableKeyboard; // Enable some options
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(m_DR->GetDevice(), m_DR->GetDeviceContext());
#endif
}

void Game::GetDefaultSize(uint32_t *width, uint32_t *height)
{
	*width = DEFAULT_WIN_WIDTH;
	*height = DEFAULT_WIN_HEIGHT;
}