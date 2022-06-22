#include "SoftShadowsConfig.h"
#include "Game.h"
#include "Utils.h"
#include "NE_Math.h"
#include "Camera.h"
#include "MeshGenerator.h"
#include "Mouse.h"

#include <d3dcompiler.h>

#if WITH_IMGUI
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#endif

#include <sstream>

using namespace Microsoft::WRL;

namespace
{
	Vec3D cubePositions[] =
	{
		{0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f},
		{-1.0f, 0.0f, -1.0f},
	};

	Vec3D cubeRotations[] =
	{
		{0.0f, 0.0f, 0.0f},
		{MathToRadians(45.0f), 0.0f, MathToRadians(45.0f)},
		{MathToRadians(15.0f), MathToRadians(15.0f), MathToRadians(15.0f)},
	};

	float cubeScales[] = {
		0.5f,
		0.5f,
		0.5f,
	};

	D3D11_RASTERIZER_DESC g_rasterizerDesc = {CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};
};

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

Actor* Game::FindActorByName(const std::string& name)
{
	auto it = std::find_if(std::begin(m_Actors), std::end(m_Actors), [&name](const Actor& actor)
		{
			return actor.GetName() == name;
		});
	return it == std::end(m_Actors) ? nullptr : &*it;
}

// TODO: Update this to get list of actors to draw
void Game::DrawScene()
{
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
	m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerFrameCB.Get(), 1);

	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerSceneCB.Get(), 2);
	m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerSceneCB.Get(), 2);

	for (size_t i = 0; i < m_Actors.size(); ++i)
	{
		Actor& actor = m_Actors[i];
		if (actor.GetName() == "Cube")
		{
			for (int i = 0; i < 3; ++i)
			{
				actor.Scale(cubeScales[i]);
				actor.Translate(cubePositions[i]);
				const float pitch = cubeRotations[i].X;
				const float yaw = cubeRotations[i].Y;
				const float roll = cubeRotations[i].Z;
				actor.Rotate(pitch, yaw, roll);
				DrawActor(actor);
			}
		}
		else
		{
			DrawActor(actor);
		}
	}
}

void Game::DrawSky()
{
	m_DR->PIXBeginEvent(L"Draw sky");
	m_Renderer.SetRasterizerState(m_CubeMap.GetRasterizerState());
	m_Renderer.SetDepthStencilState(m_CubeMap.GetDepthStencilState());
	m_Renderer.BindVertexShader(m_shaderManager.GetVertexShader("SkyVS"));
	m_Renderer.SetInputLayout(m_shaderManager.GetInputLayout());
	m_Renderer.BindPixelShader(m_shaderManager.GetPixelShader("SkyPS"));
	m_Renderer.BindShaderResource(BindTargets::PixelShader, m_CubeMap.GetCubeMap(), 0);
	m_Renderer.SetSamplerState(m_CubeMap.GetCubeMapSampler(), 0);
	m_Renderer.SetVertexBuffer(m_CubeMap.GetVertexBuffer(), m_shaderManager.GetStrides(), 0);
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
	m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
	m_Renderer.SetIndexBuffer(m_CubeMap.GetIndexBuffer(), 0);
	m_Renderer.DrawIndexed(m_CubeMap.GetNumIndices(), 0, 0);
	m_DR->PIXEndEvent();
}

void Game::CreateRasterizerState()
{
	if (FAILED(m_DR->GetDevice()->CreateRasterizerState(&g_rasterizerDesc,
		m_rasterizerState.ReleaseAndGetAddressOf())))
	{
		OutputDebugStringA("ERROR: Failed to create rasterizer state\n");
		ExitProcess(EXIT_FAILURE);
	}
}

#if WITH_IMGUI
void Game::UpdateImgui()
{
	// Any application code here
	{
		static_assert(sizeof(float) * 3 == sizeof(Vec3D), "ERROR: Cannot cast Vec3D to float[3]");
		static float zNear = m_Camera.GetZNear();
		static float zFar = m_Camera.GetZFar();
		ImGui::PushItemWidth(150.0f);
		ImGui::InputFloat("Z near", &zNear);
		ImGui::SameLine();
		ImGui::InputFloat("Z far", &zFar);
		ImGui::PopItemWidth();
		ImGui::InputFloat3("Dir light pos", reinterpret_cast<float*>(&m_PerSceneData.dirLight.Position));
		m_Camera.SetZFar(zFar);
		m_Camera.SetZNear(zNear);
	}

    if (ImGui::CollapsingHeader("Fog settings"))
    {
        ImGui::SliderFloat("Fog end", &m_PerSceneData.fogEnd, 0.0f, 100.0f);
        ImGui::SliderFloat("Fog start", &m_PerSceneData.fogStart, -10.0f, 10.0f);
        ImGui::ColorPicker4("Fog color", reinterpret_cast<float*>(&m_PerSceneData.fogColor));
    }

	if (ImGui::CollapsingHeader("Rasterizer settings"))
	{
		ImGui::PushItemWidth(150.0f);
		ImGui::SliderInt("FillMode", reinterpret_cast<int*>(&g_rasterizerDesc.FillMode), 2, 3);
		ImGui::Checkbox("FrontCounterClockwise", reinterpret_cast<bool*>(&g_rasterizerDesc.FrontCounterClockwise));
		ImGui::SliderInt("DepthBias", &g_rasterizerDesc.DepthBias, 0, 10000);
		ImGui::SliderInt("CullMode", reinterpret_cast<int*>(&g_rasterizerDesc.CullMode), 1, 3);
		ImGui::SliderFloat("DepthBiasClamp", &g_rasterizerDesc.DepthBiasClamp, -1000.0f, 1000.0f);
		ImGui::SliderFloat("SlopeScaledDepthBias", &g_rasterizerDesc.SlopeScaledDepthBias, -1000.0f, 1000.0f);
		ImGui::Checkbox("DepthClipEnable", reinterpret_cast<bool*>(&g_rasterizerDesc.DepthClipEnable));
		ImGui::Checkbox("ScissorEnable", reinterpret_cast<bool*>(&g_rasterizerDesc.ScissorEnable));
		ImGui::Checkbox("MultisampleEnable", reinterpret_cast<bool*>(&g_rasterizerDesc.MultisampleEnable));
		ImGui::Checkbox("AntialiasedLineEnable", reinterpret_cast<bool*>(&g_rasterizerDesc.AntialiasedLineEnable));
		ImGui::PopItemWidth();
		CreateRasterizerState();
	}

	{
		static std::string buffer(10240, 0);
		static std::string shaderName(256, 0);
		ImGui::PushItemWidth(150.0f);
		ImGui::InputText("Shader name", &shaderName[0], 256);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Open"))
		{
			if (shaderName.find(".hlsl") != std::string::npos)
			{
				const std::string shaderPath = UtilsFormatStr("%s/shader/%s", SRC_ROOT, shaderName.c_str());
				const auto bytes = UtilsReadData(shaderPath.c_str());
				if (buffer.size() < bytes.size())
				{
					buffer.resize(bytes.size() * 2);
					buffer.clear();
				}
				for (size_t i = 0; i < buffer.size(); ++i)
				{
					if (i < bytes.size())
					{
						buffer[i] = static_cast<char>(bytes[i]);
					}
					else
					{
						buffer[i] = 0;
					}
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Compile"))
		{
			// save new shader
			const std::string shaderPath = UtilsFormatStr("%s/shader/%s", SRC_ROOT, shaderName.c_str());
			UtilsDebugPrint("Updating shader source file %s\n", shaderPath.c_str());
			size_t sz = 0;
			for (const char& ch : buffer)
			{
				if (ch == 0)
					break;
				++sz;
			}
			UtilsWriteData(shaderPath.c_str(), &buffer[0], sz, true);
			// compile shader
			const std::string sn = shaderName.substr(0, shaderName.find(".hlsl"));
			const bool isVS = sn.find("VS") != std::string::npos;
			ComPtr<ID3DBlob> shaderBlob = nullptr;
			ComPtr<ID3DBlob> errorBlob = nullptr;
			std::wstring shaderPathW(shaderPath.size(), 0);
			std::mbstowcs(&shaderPathW[0], &shaderPath[0], shaderPathW.size());
			HR(D3DCompileFromFile(shaderPathW.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				isVS ? "vs_5_0" : "ps_5_0",
				0,
				0,
				&shaderBlob,
				&errorBlob))

				if (errorBlob.Get())
				{
					UtilsDebugPrint("ERROR: Failed to hot reload %s, because of compile error. %s\n",
						shaderPath.c_str(), static_cast<char*>(errorBlob->GetBufferPointer()));
				}
				else if (shaderBlob.Get())
				{
					if (sn.find("VS") != std::string::npos)
					{
						if (m_shaderManager.GetVertexShader(sn))
						{
							ComPtr<ID3D11VertexShader> vs = {};
							const HRESULT hr = m_DR->GetDevice()->CreateVertexShader(shaderBlob->GetBufferPointer(),
								shaderBlob->GetBufferSize(), nullptr, vs.GetAddressOf());
							UtilsDebugPrint("Hot reloading shader %s. Result: %ld\n", shaderName.c_str(), hr);
							HR(hr)
								m_shaderManager.UpdateVertexShader(sn, vs.Get());
						}
					}
					else if (sn.find("PS") != std::string::npos)
					{
						if (m_shaderManager.GetPixelShader(sn))
						{
							ComPtr<ID3D11PixelShader> ps = {};
							UtilsDebugPrint("Hot reload shader %s\n", shaderName.c_str());
							const HRESULT hr = m_DR->GetDevice()->CreatePixelShader(shaderBlob->GetBufferPointer(),
								shaderBlob->GetBufferSize(), nullptr, ps.GetAddressOf());
							UtilsDebugPrint("Hot reloading shader %s. Result: %ld\n", shaderName.c_str(), hr);
							HR(hr)
								m_shaderManager.UpdatePixelShader(sn, ps.Get());
						}
					}
				}
		}

		if (ImGui::CollapsingHeader("Shader Source"))
		{
			ImGui::InputTextMultiline("Source", &buffer[0], buffer.size(), { 640.0f, 480.0f },
				ImGuiInputTextFlags_AllowTabInput);
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		if (ImGui::CollapsingHeader(UtilsFormatStr("Cube %d", i).c_str()))
		{
			ImGui::PushItemWidth(150.0f);
			ImGui::SliderFloat(UtilsFormatStr("Cube %d scale", i).c_str(),
				&cubeScales[i], 0.1f, 2.0f);
			ImGui::SliderFloat(UtilsFormatStr("Cube %d Pos.X", i).c_str(),
				&cubePositions[i].X, -10.0f, 10.0f);
			ImGui::SameLine();
			ImGui::SliderFloat(UtilsFormatStr("Cube %d Pos.Y", i).c_str(),
				&cubePositions[i].Y, -10.0f, 10.0f);
			ImGui::SameLine();
			ImGui::SliderFloat(UtilsFormatStr("Cube %d Pos.Z", i).c_str(),
				&cubePositions[i].Z, -10.0f, 10.0f);
			ImGui::SliderAngle(UtilsFormatStr("Cube %d Pitch", i).c_str(), &cubeRotations[i].X);
			ImGui::SameLine();
			ImGui::SliderAngle(UtilsFormatStr("Cube %d Yaw", i).c_str(), &cubeRotations[i].Y);
			ImGui::SameLine();
			ImGui::SliderAngle(UtilsFormatStr("Cube %d Roll", i).c_str(), &cubeRotations[i].Z);
			ImGui::PopItemWidth();
		}
	}
}
#endif

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
	dirLight.Position = MathVec3DFromXYZ(5.0f, 5.0f, 5.0f);
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

Game::Game() :
	m_Camera{ {0.0f, 0.0f, -5.0f} }
{
	m_DR = std::make_unique<DeviceResources>();
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

	static const float CLEAR_COLOR[4] = { 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };

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
    GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerSceneConstants), &m_PerSceneData, m_PerSceneCB.Get());


	static float elapsedTime = 0.0f;
	const float deltaSeconds = static_cast<float>(m_Timer.DeltaMillis / 1000.0);
	elapsedTime += deltaSeconds;

	if (elapsedTime >= 1.0f)
	{
		SetWindowText(m_DR->GetWindow(),
			UtilsFormatStr("soft-shadows -- FPS: %d, frame: %f s",
				static_cast<int>(elapsedTime / deltaSeconds),
				deltaSeconds).c_str());
		elapsedTime = 0.0f;
	}

#if WITH_IMGUI
	static float totalTime = 0.0f;
	totalTime += deltaSeconds;
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
	m_Renderer.SetBlendState(nullptr);
	m_Renderer.SetDepthStencilState(nullptr);
	m_Renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
	m_Renderer.SetRasterizerState(m_rasterizerState.Get());
	m_Renderer.SetSamplerState(m_DefaultSampler.Get(), 0);

	m_DR->PIXBeginEvent(L"Shadow pass");
	{
		m_ShadowMap.Bind(m_DR->GetDeviceContext());
		Mat4X4 view = {};
		Mat4X4 proj = {};
		BuildShadowTransform(view, proj);
		m_Renderer.BindPixelShader(nullptr);
		m_Renderer.BindVertexShader(m_shaderManager.GetVertexShader("ShadowVS"));
		m_Renderer.SetInputLayout(m_shaderManager.GetInputLayout());
		m_Renderer.SetSamplerState(m_ShadowMap.GetShadowSampler(), 1);
		m_PerFrameData.proj = proj;
		m_PerFrameData.view = view;
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());
		DrawScene();
		m_ShadowMap.Unbind(m_DR->GetDeviceContext());
	}
	m_DR->PIXEndEvent();

	m_DR->PIXBeginEvent(L"Color pass");
	// reset view proj matrix back to camera
	{
		m_Renderer.Clear();
		m_PerFrameData.view = m_Camera.GetViewMat();
		m_PerFrameData.proj = m_Camera.GetProjMat();
		m_PerFrameData.cameraPosW = m_Camera.GetPos();
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());
		m_Renderer.BindVertexShader(m_shaderManager.GetVertexShader("ColorVS"));
		m_Renderer.BindPixelShader(m_shaderManager.GetPixelShader("PhongPS"));
		m_Renderer.SetInputLayout(m_shaderManager.GetInputLayout());
		m_Renderer.BindShaderResource(BindTargets::PixelShader, m_ShadowMap.GetDepthMapSRV(), 4);

		DrawScene();
	}
	m_DR->PIXEndEvent();
	m_DR->PIXBeginEvent(L"Light source pass");
	// Light properties
	//for (uint32_t i = 0; i < _countof(m_PerSceneData.pointLights); ++i)
	{
		m_PerObjectData.world = MathMat4X4TranslateFromVec3D(&m_PerSceneData.dirLight.Position);
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
			sizeof(PerObjectConstants),
			&m_PerObjectData,
			m_PerObjectCB.Get());

		m_Renderer.BindPixelShader(m_shaderManager.GetPixelShader("LightPS"));
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);
		const auto sphere = FindActorByName("Sphere");
		m_Renderer.SetIndexBuffer(sphere->GetIndexBuffer(), 0);
		m_Renderer.SetVertexBuffer(sphere->GetVertexBuffer(), m_shaderManager.GetStrides(), 0);
		m_Renderer.DrawIndexed(sphere->GetNumIndices(), 0, 0);
	}
	m_DR->PIXEndEvent();
	//// TODO: Need to have a reflection mechanism to query amount of SRV that is possible to bind to PS
	//// After this this clear code could be placed to Renderer::Clear
	//ID3D11ShaderResourceView* nullSRVs[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	//m_Renderer.BindShaderResources(BindTargets::PixelShader, nullSRVs, 7);

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
		Material material(
			{ 0.23125f,0.23125f,0.23125f,1.0f },
			{ 0.2775f,0.2775f,0.2775f,1.0f },
			{ 0.773911f,0.773911f,0.773911f,89.6f },
			{ 0.5f,0.5f,0.5f,1.0f }
		);
		actor.SetMaterial(&material);
		actor.SetName("Sphere");
		actor.SetIsVisible(false);
		m_Actors.emplace_back(actor);
	}

	const Material material = {
			{0.24725f, 0.1995f, 0.0745f, 1.0f},
			{0.75164f, 0.60648f, 0.22648f, 1.0f},
			{0.628281f, 0.555802f, 0.366065f, 32.0f},
			{0.0f, 0.0f, 0.0f, 0.0f}
		};

	// load cube
	{
		Actor actor = Actor();
		actor.LoadModel(UtilsFormatStr("%s/meshes/cube.obj", ASSETS_ROOT).c_str());
		actor.CreateIndexBuffer(m_DR->GetDevice());
		actor.CreateVertexBuffer(m_DR->GetDevice());
		actor.SetName("Cube");
		actor.SetIsVisible(true);
		actor.SetMaterial(&material);
		m_Actors.emplace_back(actor);
	}

	// load plane
	{
		const Vec3D origin = { 0.0f, 0.0f, 0.0f };
		std::unique_ptr<Mesh> mesh = MGGeneratePlane(&origin, 10.0f, 10.0f);
		Actor plane = Actor(mesh.get());
		plane.CreateIndexBuffer(m_DR->GetDevice());
		plane.CreateVertexBuffer(m_DR->GetDevice());
		const Vec3D offset = { 0.0f, -1.0f, 0.0f };
		plane.Translate(offset);

		plane.LoadTexture(UtilsFormatStr("%s/textures/wood_floor_diffuseOriginal.jpg", ASSETS_ROOT).c_str(), TextureType::Diffuse, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/wood_floor_normal.jpg", ASSETS_ROOT).c_str(), TextureType::Normal, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/wood_floor_smoothness.jpg", ASSETS_ROOT).c_str(), TextureType::Gloss, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.LoadTexture(UtilsFormatStr("%s/textures/wood_floor_metallic.jpg", ASSETS_ROOT).c_str(), TextureType::Specular, m_DR->GetDevice(), m_DR->GetDeviceContext());
		plane.SetMaterial(&material);
		plane.SetIsVisible(true);
		plane.SetName("Plane");
		m_Actors.emplace_back(plane);
	}

	Actor* p = FindActorByName("Plane");
	Actor* c = FindActorByName("Cube");
	c->SetTextures(p->GetShaderResources());
}

void Game::Initialize(HWND hWnd, uint32_t width, uint32_t height)
{
	using namespace Microsoft::WRL;
#ifdef MATH_TEST
	MathTest();
#endif
	m_DR->SetWindow(hWnd, width, height);
	m_DR->CreateDeviceResources();
	m_DR->CreateWindowSizeDependentResources();
	TimerInitialize(&m_Timer);
	Mouse::Get().SetWindowDimensions(m_DR->GetBackBufferWidth(), m_DR->GetBackBufferHeight());
	ID3D11Device* device = m_DR->GetDevice();

	m_ShadowMap.InitResources(device, 1024, 1024);
	m_Camera.SetViewDimensions(m_DR->GetBackBufferWidth(), m_DR->GetBackBufferHeight());
	m_CubeMap.LoadCubeMap(device, {
		UtilsFormatStr("%s/textures/right.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/left.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/top.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/bottom.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/front.jpg", ASSETS_ROOT).c_str(),
		UtilsFormatStr("%s/textures/back.jpg", ASSETS_ROOT).c_str(),
	});

	m_dynamicCubeMap.Init(device);
	m_dynamicCubeMap.BuildCubeFaceCamera({ 0.0f, 0.0f, 0.0f });
	// init actors
	CreateActors();
	m_CubeMap.CreateCube(*FindActorByName("Cube"), device);
	InitPerSceneConstants();
	m_shaderManager.Initialize(device, SHADERS_ROOT, UtilsFormatStr("%s/shader", SRC_ROOT));

	GameCreateConstantBuffer(device, sizeof(PerSceneConstants), &m_PerSceneCB);
	GameCreateConstantBuffer(device, sizeof(PerObjectConstants), &m_PerObjectCB);
	GameCreateConstantBuffer(device, sizeof(PerFrameConstants), &m_PerFrameCB);
	GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerSceneConstants), &m_PerSceneData, m_PerSceneCB.Get());

	CreateDefaultSampler();
	CreateRasterizerState();

	m_Renderer.SetDeviceResources(m_DR.get());

#if WITH_IMGUI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	const std::string droidSansTtf = UtilsFormatStr("%s/../imgui-1.87/misc/fonts/DroidSans.ttf", SRC_ROOT);
	io.Fonts->AddFontFromFileTTF(droidSansTtf.c_str(), 16.0f);

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(m_DR->GetDevice(), m_DR->GetDeviceContext());
#endif
}

void Game::GetDefaultSize(uint32_t* width, uint32_t* height)
{
	*width = DEFAULT_WIN_WIDTH;
	*height = DEFAULT_WIN_HEIGHT;
}

void Game::BuildShadowTransform(Mat4X4& view, Mat4X4& proj)
{
	// Only the first "main" light casts a shadow.
	const Vec3D lightPos = m_PerSceneData.dirLight.Position;
	const Vec3D targetPos = { 0.0f, 0.0f, 0.0f };
	const Vec3D worldUp = { 0.0f, 1.0f, 0.0f };
	const float radius = MathVec3DLength(lightPos);

	const Vec3D right = (targetPos - lightPos).Cross(worldUp);
	const Vec3D up = right.Cross(targetPos - lightPos);

	view = MathMat4X4ViewAt(&lightPos, &targetPos, &up);
	proj = MathMat4X4OrthographicOffCenter(-radius, radius, -radius, radius, m_Camera.GetZNear(), m_Camera.GetZFar());
}


void Game::DrawActor(const Actor& actor)
{
	if (actor.IsVisible())
	{
		m_Renderer.BindShaderResources(BindTargets::PixelShader, actor.GetShaderResources(), ACTOR_NUM_TEXTURES);
		m_PerObjectData.material = actor.GetMaterial();
		m_PerObjectData.world = actor.GetWorld();
		m_PerObjectData.worldInvTranspose = MathMat4X4Inverse(actor.GetWorld());
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(),
			sizeof(PerObjectConstants),
			&m_PerObjectData,
			m_PerObjectCB.Get());
		Mat4X4 view = {};
		Mat4X4 proj = {};
		BuildShadowTransform(view, proj);
		const Mat4X4 toLightSpace = actor.GetWorld() * view * proj;
		m_PerFrameData.shadowTransform = toLightSpace;
		GameUpdateConstantBuffer(m_DR->GetDeviceContext(), sizeof(PerFrameConstants), &m_PerFrameData, m_PerFrameCB.Get());
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerObjectCB.Get(), 0);
		m_Renderer.BindConstantBuffer(BindTargets::VertexShader, m_PerFrameCB.Get(), 1);
		m_Renderer.BindConstantBuffer(BindTargets::PixelShader, m_PerFrameCB.Get(), 1);

		m_Renderer.SetIndexBuffer(actor.GetIndexBuffer(), 0);
		m_Renderer.SetVertexBuffer(actor.GetVertexBuffer(), m_shaderManager.GetStrides(), 0);

		m_Renderer.DrawIndexed(actor.GetNumIndices(), 0, 0);
	}
}

void Game::OnWindowSizeChanged(int width, int height)
{
	if (!m_DR->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();
}

void Game::CreateWindowSizeDependentResources()
{
	auto const size = m_DR->GetOutputSize();
	const float aspectRatio = static_cast<float>(size.right) / static_cast<float>(size.bottom);
	float fovAngleY = 45.0f;

	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	m_Camera.SetFov(fovAngleY);
	m_Camera.SetViewDimensions(size.right, size.bottom);

}
