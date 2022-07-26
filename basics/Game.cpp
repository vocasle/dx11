#include "Game.h"
#include "BasicsConfig.h"
#include "NE_Math.h"
#include "Utils.h"
#include "objloader.h"

#if WITH_IMGUI
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#endif

using namespace Microsoft::WRL;

namespace
{
static Vec3D cubePositions[] = {
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f },
	{ -1.0f, 0.0f, -1.0f },
};

static Vec3D cubeRotations[] = {
	{ 0.0f, 0.0f, 0.0f },
	{ MathToRadians(45.0f), 0.0f, MathToRadians(45.0f) },
	{ MathToRadians(15.0f), MathToRadians(15.0f), MathToRadians(15.0f) },
};

static float cubeScales[] = {
	0.5f,
	0.5f,
	0.5f,
};

bool rotateDirLight = false;

struct Vertex {
	Vec3D Position;
	Vec3D Normal;
	Vec2D TexCoord;
};

std::vector<Vertex> g_Vertices;
std::vector<uint32_t> g_Indices;
ComPtr<ID3D11Buffer> g_VB;
ComPtr<ID3D11Buffer> g_IB;

std::unique_ptr<Model> g_Cube = nullptr;
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

#if WITH_IMGUI
void Game::UpdateImgui()
{
	// Any application code here
	ImGui::Checkbox("Rotate dir light", &rotateDirLight);
	static float zNear = 0.5f;
	ImGui::SliderFloat("Z near", &zNear, 0.1f, 10.0f, "%f", 1.0f);
	static float zFar = 200.0f;
	ImGui::SliderFloat("Z far", &zFar, 10.0f, 1000.0f, "%f", 1.0f);

	if (ImGui::CollapsingHeader("Cube 0")) {
		ImGui::SliderFloat("Cube 0 scale", cubeScales, 0.1f, 2.0f, "%f",
				   1.0f);
		ImGui::SliderFloat("Cube 0 Pos.X", &cubePositions[0].X, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderFloat("Cube 0 Pos.Y", &cubePositions[0].Y, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderFloat("Cube 0 Pos.Z", &cubePositions[0].Z, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderAngle("Cube 0 Pitch", &cubeRotations[0].X);
		ImGui::SliderAngle("Cube 0 Yaw", &cubeRotations[0].Y);
		ImGui::SliderAngle("Cube 0 Roll", &cubeRotations[0].Z);
	}

	if (ImGui::CollapsingHeader("Cube 1")) {
		ImGui::SliderFloat("Cube 1 scale", &cubeScales[1], 0.1f, 2.0f,
				   "%f", 1.0f);

		ImGui::SliderFloat("Cube 1 Pos.X", &cubePositions[1].X, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderFloat("Cube 1 Pos.Y", &cubePositions[1].Y, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderFloat("Cube 1 Pos.Z", &cubePositions[1].Z, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderAngle("Cube 1 Pitch", &cubeRotations[1].X);
		ImGui::SliderAngle("Cube 1 Yaw", &cubeRotations[1].Y);
		ImGui::SliderAngle("Cube 1 Roll", &cubeRotations[1].Z);
	}

	if (ImGui::CollapsingHeader("Cube 2")) {
		ImGui::SliderFloat("Cube 2 scale", &cubeScales[2], 0.1f, 2.0f,
				   "%f", 1.0f);
		ImGui::SliderFloat("Cube 2 Pos.X", &cubePositions[2].X, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderFloat("Cube 2 Pos.Y", &cubePositions[2].Y, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderFloat("Cube 2 Pos.Z", &cubePositions[2].Z, -10.0f,
				   10.0f, "%f", 1.0f);
		ImGui::SliderAngle("Cube 2 Pitch", &cubeRotations[2].X);
		ImGui::SliderAngle("Cube 2 Yaw", &cubeRotations[2].Y);
		ImGui::SliderAngle("Cube 2 Roll", &cubeRotations[2].Z);
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

Game::Game()
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
	static float elapsedTime = 0.0f;
	const float deltaSeconds =
		static_cast<float>(m_Timer.DeltaMillis / 1000.0);
	elapsedTime += deltaSeconds;

	if (elapsedTime >= 1.0f) {
		D3D11_VIEWPORT viewport = m_DR->GetViewport();
		SetWindowText(
			m_DR->GetWindow(),
			UtilsFormatStr(
				"basics -- FPS: %d, frame: %.4f s, %.0fx%.0f",
				static_cast<int>(elapsedTime / deltaSeconds),
				deltaSeconds, viewport.Width, viewport.Height)
				.c_str());
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
	m_Renderer.SetRasterizerState(m_DR->GetRasterizerState());
	m_Renderer.SetSamplerState(m_DefaultSampler.Get(), 0);
	m_Renderer.SetVertexBuffer(g_VB.Get(), sizeof(Vertex), 0);
	m_Renderer.SetIndexBuffer(g_IB.Get(), 0);
	m_Renderer.BindVertexShader(m_VS.Get());
	m_Renderer.BindPixelShader(m_PS.Get());
	m_Renderer.SetInputLayout(m_IL.Get());
	m_Renderer.DrawIndexed(g_Indices.size(), 0, 0);

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

	// CreatePixelShader("ParticlePS.cso", device,
	// m_particlePS.ReleaseAndGetAddressOf()); auto bytes =
	// CreateVertexShader("VertexShader.cso", device,
	// m_VS.ReleaseAndGetAddressOf());

	// GameCreateConstantBuffer(device, sizeof(PerSceneConstants),
	// &m_PerSceneCB);
	CreateDefaultSampler();

	m_Renderer.SetDeviceResources(m_DR.get());

	g_Cube = OLLoad(
		UtilsFormatStr("%s/meshes/cube.obj", ASSETS_ROOT).c_str());

	for (int i = 0; const Face &face : g_Cube->Meshes[0].Faces) {
		g_Indices.push_back(i++);
		Vertex v = {};
		v.Position = g_Cube->Meshes[0].Positions[face.posIdx];
		v.Normal = g_Cube->Meshes[0].Normals[face.normIdx];
		v.TexCoord = g_Cube->Meshes[0].TexCoords[face.texIdx];
		g_Vertices.push_back(v);
	}

	UtilsCreateVertexBuffer(device, &g_Vertices[0], g_Vertices.size(),
				sizeof(Vertex), g_VB.ReleaseAndGetAddressOf());
	UtilsCreateIndexBuffer(device, &g_Indices[0], g_Indices.size(),
			       g_IB.ReleaseAndGetAddressOf());

	CreatePixelShader("BasicPS.cso", device, m_PS.ReleaseAndGetAddressOf());
	auto bytes = CreateVertexShader("BasicVS.cso", device,
					m_VS.ReleaseAndGetAddressOf());
	UtilsDebugPrint("bytes=%llu\n", bytes.size());

	{
		D3D11_INPUT_ELEMENT_DESC desc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			  offsetof(Vertex, Position),
			  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			  offsetof(Vertex, Normal), D3D11_INPUT_PER_VERTEX_DATA,
			  0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			  offsetof(Vertex, TexCoord),
			  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(device->CreateInputLayout(
			desc, sizeof(desc) / sizeof(desc[0]), &bytes[0],
			bytes.size(), m_IL.ReleaseAndGetAddressOf()))
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
