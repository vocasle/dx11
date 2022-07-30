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
        UtilsFatalError(
            "ERROR: Failed to create per frame constants cbuffer\n");
    }
}

void
Game::CreateRasterizerState() {
    throw std::runtime_error("Not implemented");
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
        SetWindowText(
            m_deviceResources->GetWindow(),
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

    static constexpr float CLEAR_COLOR[] = {0.2f, 0.2f, 0.2f, 1};
    m_renderer.SetRenderTargets(m_deviceResources->GetRenderTargetView(),
                                m_deviceResources->GetDepthStencilView());
    m_renderer.SetViewport(m_deviceResources->GetViewport());
    m_renderer.Clear(CLEAR_COLOR);
    m_renderer.SetBlendState(nullptr);
    m_renderer.SetDepthStencilState(nullptr);
    m_renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
    m_renderer.SetRasterizerState(m_rasterizerState.Get());
    m_renderer.SetSamplerState(m_defaultSampler.Get(), 0);

    m_deviceResources->PIXBeginEvent(L"Color pass");
    // reset view proj matrix back to camera
    {
        // m_PerFrameData.view = m_camera.GetViewMat();
        // m_PerFrameData.proj = m_camera.GetProjMat();
        // m_PerFrameData.cameraPosW = m_camera.GetPos();
        // GameUpdateConstantBuffer(m_deviceResources->GetDeviceContext(),
        // 			 sizeof(PerFrameConstants),
        // 			 &m_PerFrameData, m_PerFrameCB.Get());
        m_renderer.BindVertexShader(m_shaderManager.GetVertexShader("ColorVS"));
        m_renderer.BindPixelShader(m_shaderManager.GetPixelShader("PhongPS"));
        m_renderer.SetSamplerState(m_defaultSampler.Get(), 0);
        m_renderer.SetInputLayout(m_shaderManager.GetInputLayout());

        for (const Mesh &mesh : m_meshes) {
          m_renderer.SetVertexBuffer(mesh.GetVertexBuffer(), mesh.GetVertexSize(), 0);
          m_renderer.SetIndexBuffer(mesh.GetIndexBuffer(), 0);
          m_renderer.DrawIndexed(mesh.GetIndexCount(), 0, 0);
        }
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

    m_shaderManager.Initialize(
        device, SHADERS_ROOT, UtilsFormatStr("%s/shader", SOURCE_ROOT));

    CreateDefaultSampler();
    // CreateRasterizerState();

    m_renderer.SetDeviceResources(m_deviceResources.get());

    m_meshes = m_modelLoader.Load(UtilsFormatStr("%s/sponza.glb", SPONZA_ROOT));

    for (Mesh &mesh : m_meshes) mesh.CreateBuffers(device);

#if WITH_IMGUI
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    const std::string droidSansTtf = UtilsFormatStr(
        "%s/../imgui-1.87/misc/fonts/DroidSans.ttf", SOURCE_ROOT);
    io.Fonts->AddFontFromFileTTF(droidSansTtf.c_str(), 16.0f);

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(m_deviceResources->GetDevice(),
                        m_deviceResources->GetDeviceContext());
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
}
