#include "SponzaGame.h"

#include <d3dcompiler.h>

#include "Camera.h"
#include "DynamicConstBuffer.h"
#include "Mouse.h"
#include "NE_Math.h"
#include "Texture.h"
#include "Utils.h"

#if WITH_IMGUI
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#endif

#include <stdexcept>

using namespace Microsoft::WRL;

namespace {
Vec3D cubePositions[] = {
    {2.794f, 0.441f, 0.294f},
    {-0.735f, 1.324f, 3.088f},
    {-1.0f, -1.324f, -1.765f},
};

Vec3D cubeRotations[] = {
    {MathToRadians(-42.0f), MathToRadians(53.0f), 0.0f},
    {MathToRadians(45.0f), 0.0f, MathToRadians(45.0f)},
    {MathToRadians(11.0f), MathToRadians(-37.0f), MathToRadians(15.0f)},
};

float cubeScales[] = {
    0.854f,
    0.8f,
    1.357f,
};

Vec4D g_PointLightColors[] = {
    {1, 1, 0, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, {1, 1, 0, 1}};

D3D11_RASTERIZER_DESC g_rasterizerDesc = {
    CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};

std::unique_ptr<Texture> g_OffscreenRTV;
std::unique_ptr<Texture> g_BrightessRTV;
std::unique_ptr<Texture> g_BlurRTV;
std::unique_ptr<Texture> g_BlurRTV2;
std::unique_ptr<DynamicConstBuffer> g_FogCBuf;
std::unique_ptr<DynamicConstBuffer> g_LightCBuf;
std::unique_ptr<DynamicConstBuffer> g_BlurCBuf;

struct BloomSettings {
  bool isEnabled;
  bool isOnlyBrightness;
  bool isOnlyColor;
  bool isOnlyBlur;
  float threshold;
};

BloomSettings g_BloomSettings = {false, false, false, false, 1.0f};
};  // namespace

static void
GameUpdateConstantBuffer(ID3D11DeviceContext *context,
                         size_t bufferSize,
                         void *data,
                         ID3D11Buffer *dest) {
  D3D11_MAPPED_SUBRESOURCE mapped = {};

  if (FAILED(context->Map(
          (ID3D11Resource *)dest, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    UtilsFatalError("ERROR: Failed to map constant buffer\n");
  }
  memcpy(mapped.pData, data, bufferSize);
  context->Unmap((ID3D11Resource *)dest, 0);
}

static void
GameCreateConstantBuffer(ID3D11Device *device,
                         size_t byteWidth,
                         ID3D11Buffer **pDest) {
  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bufferDesc.ByteWidth = byteWidth;
  bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

  if (FAILED(device->CreateBuffer(&bufferDesc, NULL, pDest))) {
    UtilsFatalError("ERROR: Failed to create per frame constants cbuffer\n");
  }
}

void
Game::CreateRasterizerState() {
  if (FAILED(m_deviceResources->GetDevice()->CreateRasterizerState(
          &g_rasterizerDesc, m_rasterizerState.ReleaseAndGetAddressOf()))) {
    OutputDebugStringA("ERROR: Failed to create rasterizer state\n");
    ExitProcess(EXIT_FAILURE);
  }
}

#if WITH_IMGUI
void
Game::UpdateImgui() {
  if (ImGui::Button("Compile")) {
    m_shaderManager.Recompile(m_deviceResources->GetDevice());
  }
}
#endif

void
Game::CreateDefaultSampler() {
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

  if (FAILED(m_deviceResources->GetDevice()->CreateSamplerState(
          &samplerDesc, m_defaultSampler.ReleaseAndGetAddressOf()))) {
    UtilsDebugPrint("ERROR: Failed to create default sampler state\n");
    ExitProcess(EXIT_FAILURE);
  }
}

Game::Game()
    : m_camera{{0.0f, 0.0f, -5.0f}} {
  m_deviceResources = std::make_unique<DeviceResources>();
}

Game::~Game() {
#if WITH_IMGUI
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
#endif
}

void
Game::Clear() {
  ID3D11DeviceContext *ctx = m_deviceResources->GetDeviceContext();
  ID3D11RenderTargetView *rtv = m_deviceResources->GetRenderTargetView();
  ID3D11DepthStencilView *dsv = m_deviceResources->GetDepthStencilView();

  static const float CLEAR_COLOR[4] = {
      0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f};

  ctx->Flush();

  ctx->ClearRenderTargetView(rtv, CLEAR_COLOR);
  ctx->ClearDepthStencilView(
      dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  ctx->OMSetRenderTargets(1, &rtv, dsv);
  ctx->RSSetViewports(1, &m_deviceResources->GetViewport());
}

void
Game::Update() {
  m_camera.ProcessKeyboard(m_timer.DeltaMillis);
  m_camera.ProcessMouse(m_timer.DeltaMillis);

  // TODO: Replace with dynamic const buffer
  // m_PerFrameData.view = m_camera.GetViewMat();
  // m_PerFrameData.proj = m_camera.GetProjMat();
  // m_PerFrameData.cameraPosW = m_camera.GetPos();

  // GameUpdateConstantBuffer(m_deviceResources->GetDeviceContext(),
  // 			 sizeof(PerFrameConstants), &m_PerFrameData,
  // 			 m_PerFrameCB.Get());
  // GameUpdateConstantBuffer(m_deviceResources->GetDeviceContext(),
  // 			 sizeof(PerSceneConstants), &m_PerSceneData,
  // 			 m_PerSceneCB.Get());

  static float elapsedTime = 0.0f;
  const float deltaSeconds = static_cast<float>(m_timer.DeltaMillis / 1000.0);
  elapsedTime += deltaSeconds;

  if (elapsedTime >= 1.0f) {
    SetWindowText(m_deviceResources->GetWindow(),
                  UtilsFormatStr("sponza -- FPS: %d, frame: %f s",
                                 static_cast<int>(elapsedTime / deltaSeconds),
                                 deltaSeconds)
                      .c_str());
    elapsedTime = 0.0f;
  }
}

void
Game::Render() {
#if WITH_IMGUI
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
#endif

#if WITH_IMGUI
  UpdateImgui();
#endif

  static constexpr float BLACK_COLOR[] = {0, 0, 0, 1};
  m_renderer.SetRenderTargets(m_deviceResources->GetRenderTargetView(),
                              m_deviceResources->GetDepthStencilView());
  m_renderer.SetViewport(m_deviceResources->GetViewport());
  m_renderer.Clear(nullptr);
  m_renderer.SetBlendState(nullptr);
  m_renderer.SetDepthStencilState(nullptr);
  m_renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
  m_renderer.SetRasterizerState(m_rasterizerState.Get());
  m_renderer.SetSamplerState(m_defaultSampler.Get(), 0);

  m_deviceResources->PIXBeginEvent(L"Color pass");
  // reset view proj matrix back to camera
  {
    // draw to offscreen texture
    // m_renderer.SetRenderTargets(g_OffscreenRTV->GetRTV(),
    // 			    g_OffscreenRTV->GetDSV());
    // m_renderer.Clear(nullptr);
    // m_PerFrameData.view = m_camera.GetViewMat();
    // m_PerFrameData.proj = m_camera.GetProjMat();
    // m_PerFrameData.cameraPosW = m_camera.GetPos();
    // GameUpdateConstantBuffer(m_deviceResources->GetDeviceContext(),
    // 			 sizeof(PerFrameConstants),
    // 			 &m_PerFrameData, m_PerFrameCB.Get());
    // m_renderer.BindVertexShader(
    // 	m_shaderManager.GetVertexShader("ColorVS"));
    // m_renderer.BindPixelShader(
    // 	m_shaderManager.GetPixelShader("PhongPS"));
    // m_renderer.SetSamplerState(m_defaultSampler.Get(), 0);
    // m_renderer.SetSamplerState(m_ShadowMap.GetShadowSampler(), 1);
    // m_renderer.SetInputLayout(m_shaderManager.GetInputLayout());
    // m_renderer.BindShaderResource(BindTargets::PixelShader,
    // 			      m_ShadowMap.GetDepthMapSRV(), 4);

    // DrawScene();
  }
  m_deviceResources->PIXEndEvent();

#if WITH_IMGUI
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

  m_renderer.Present();
}

void
Game::Tick() {
  TimerTick(&m_timer);
  Update();
  Render();
}

void
Game::Initialize(HWND hWnd, uint32_t width, uint32_t height) {
  using namespace Microsoft::WRL;
#ifdef MATH_TEST
  MathTest();
#endif
  m_deviceResources->SetWindow(hWnd, width, height);
  m_deviceResources->CreateDeviceResources();
  m_deviceResources->CreateWindowSizeDependentResources();
  TimerInitialize(&m_timer);
  Mouse::Get().SetWindowDimensions(m_deviceResources->GetBackBufferWidth(),
                                   m_deviceResources->GetBackBufferHeight());
  ID3D11Device *device = m_deviceResources->GetDevice();

  // m_ShadowMap.InitResources(device, 1024, 1024);
  // m_camera.SetViewDimensions(m_deviceResources->GetBackBufferWidth(),
  // 			   m_deviceResources->GetBackBufferHeight());
  // m_CubeMap.LoadCubeMap(
  // 	device,
  // 	{
  // 		UtilsFormatStr("%s/textures/right.jpg", ASSETS_ROOT)
  // 			.c_str(),
  // 		UtilsFormatStr("%s/textures/left.jpg", ASSETS_ROOT)
  // 			.c_str(),
  // 		UtilsFormatStr("%s/textures/top.jpg", ASSETS_ROOT)
  // 			.c_str(),
  // 		UtilsFormatStr("%s/textures/bottom.jpg", ASSETS_ROOT)
  // 			.c_str(),
  // 		UtilsFormatStr("%s/textures/front.jpg", ASSETS_ROOT)
  // 			.c_str(),
  // 		UtilsFormatStr("%s/textures/back.jpg", ASSETS_ROOT)
  // 			.c_str(),
  // 	});

  // m_dynamicCubeMap.Init(device);
  // m_dynamicCubeMap.BuildCubeFaceCamera({ 0.0f, 0.0f, 0.0f });

  m_shaderManager.Initialize(
      device, SHADERS_ROOT, UtilsFormatStr("%s/shader", SOURCE_ROOT));

  // GameCreateConstantBuffer(device, sizeof(PerSceneConstants),
  // 			 &m_PerSceneCB);
  // GameCreateConstantBuffer(device, sizeof(PerObjectConstants),
  // 			 &m_PerObjectCB);
  // GameCreateConstantBuffer(device, sizeof(PerFrameConstants),
  // 			 &m_PerFrameCB);
  // GameUpdateConstantBuffer(m_deviceResources->GetDeviceContext(),
  // 			 sizeof(PerSceneConstants), &m_PerSceneData,
  // 			 m_PerSceneCB.Get());

  CreateDefaultSampler();
  g_rasterizerDesc.DepthBias = 500;
  CreateRasterizerState();

  m_renderer.SetDeviceResources(m_deviceResources.get());

  g_OffscreenRTV = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                             m_deviceResources->GetOutputSize().right,
                                             m_deviceResources->GetOutputSize().bottom,
                                             m_deviceResources->GetDevice());

  g_BrightessRTV = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                             m_deviceResources->GetOutputSize().right,
                                             m_deviceResources->GetOutputSize().bottom,
                                             m_deviceResources->GetDevice());

  g_BlurRTV = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                        m_deviceResources->GetOutputSize().right,
                                        m_deviceResources->GetOutputSize().bottom,
                                        m_deviceResources->GetDevice());

  g_BlurRTV2 = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                         m_deviceResources->GetOutputSize().right,
                                         m_deviceResources->GetOutputSize().bottom,
                                         m_deviceResources->GetDevice());

  {
    DynamicConstBufferDesc desc = {};
    desc.AddNode({"fogEnd", NodeType::Float});
    desc.AddNode({"fogStart", NodeType::Float});
    desc.AddNode({"width", NodeType::Float});
    desc.AddNode({"height", NodeType::Float});
    desc.AddNode({"fogColor", NodeType::Float4});
    desc.AddNode({"world", NodeType::Float4X4});
    desc.AddNode({"viewInverse", NodeType::Float4X4});
    desc.AddNode({"projInverse", NodeType::Float4X4});
    desc.AddNode({"cameraPos", NodeType::Float3});
    desc.AddNode({"_pad1", NodeType::Float});
    g_FogCBuf = std::make_unique<DynamicConstBuffer>(desc);
    g_FogCBuf->SetValue("width", m_deviceResources->GetOutputSize().right);
    g_FogCBuf->SetValue("height", m_deviceResources->GetOutputSize().bottom);
    g_FogCBuf->CreateConstantBuffer(m_deviceResources->GetDevice());
  }

  {
    DynamicConstBufferDesc desc = {};
    desc.AddNode({"color", NodeType::Float4});
    // desc.AddNode({"world", NodeType::Float4X4});
    // desc.AddNode({"view", NodeType::Float4X4});
    // desc.AddNode({"projection", NodeType::Float4X4});
    g_LightCBuf = std::make_unique<DynamicConstBuffer>(desc);
    g_LightCBuf->CreateConstantBuffer(m_deviceResources->GetDevice());
  }

  {
    DynamicConstBufferDesc desc = {};
    desc.AddNode({"width", NodeType::Float});
    desc.AddNode({"height", NodeType::Float});
    desc.AddNode({"isHorizontal", NodeType::Bool});
    desc.AddNode({"pad", NodeType::Float});
    g_BlurCBuf = std::make_unique<DynamicConstBuffer>(desc);
    g_BlurCBuf->CreateConstantBuffer(m_deviceResources->GetDevice());
  }

#if WITH_IMGUI
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  const std::string droidSansTtf =
      UtilsFormatStr("%s/../imgui-1.87/misc/fonts/DroidSans.ttf", SOURCE_ROOT);
  io.Fonts->AddFontFromFileTTF(droidSansTtf.c_str(), 16.0f);

  ImGui_ImplWin32_Init(hWnd);
  ImGui_ImplDX11_Init(m_deviceResources->GetDevice(), m_deviceResources->GetDeviceContext());
#endif
}

void
Game::GetDefaultSize(uint32_t *width, uint32_t *height) {
  *width = DEFAULT_WIN_WIDTH;
  *height = DEFAULT_WIN_HEIGHT;
}

void
Game::OnWindowSizeChanged(int width, int height) {
  if (!m_deviceResources->WindowSizeChanged(width, height))
    return;

  CreateWindowSizeDependentResources();
}

void
Game::CreateWindowSizeDependentResources() {
  const auto size = m_deviceResources->GetOutputSize();
  const float aspectRatio =
      static_cast<float>(size.right) / static_cast<float>(size.bottom);
  float fovAngleY = 45.0f;

  // portrait or snapped view.
  if (aspectRatio < 1.0f) {
    fovAngleY *= 2.0f;
  }

  m_camera.SetFov(fovAngleY);
  m_camera.SetViewDimensions(size.right, size.bottom);

  g_OffscreenRTV = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                             m_deviceResources->GetOutputSize().right,
                                             m_deviceResources->GetOutputSize().bottom,
                                             m_deviceResources->GetDevice());
  g_BrightessRTV = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                             m_deviceResources->GetOutputSize().right,
                                             m_deviceResources->GetOutputSize().bottom,
                                             m_deviceResources->GetDevice());
  g_BlurRTV = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                        m_deviceResources->GetOutputSize().right,
                                        m_deviceResources->GetOutputSize().bottom,
                                        m_deviceResources->GetDevice());
  g_BlurRTV2 = std::make_unique<Texture>(DXGI_FORMAT_B8G8R8A8_UNORM,
                                         m_deviceResources->GetOutputSize().right,
                                         m_deviceResources->GetOutputSize().bottom,
                                         m_deviceResources->GetDevice());
}
