#include "FogGame.h"
#include "Config.h"
#include "objloader.h"
#include "Utils.h"
#include "NE_Math.h"
#include "Image.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"


#include <vector>
#include <memory>
#include <wrl/client.h>

using namespace Microsoft::WRL;

struct Drawable
{
	Drawable() {}
	Drawable(const Model* model, ID3D11Device* device)
	{
		size_t i = 0;
		for (const Mesh& mesh : model->Meshes)
		{
			for (const Face& face : mesh.Faces)
			{
				Vertices.emplace_back(mesh.Positions[face.posIdx], mesh.Normals[face.normIdx], mesh.TexCoords[face.normIdx]);
				Indices.emplace_back(i++);
			}
		}

		UtilsCreateVertexBuffer(device, &Vertices[0], Vertices.size(),
			sizeof(Vertex), VertexBuffer.ReleaseAndGetAddressOf());
		UtilsCreateIndexBuffer(device, &Indices[0], Indices.size(),
			IndexBuffer.ReleaseAndGetAddressOf());
	}
	struct Vertex
	{
		Vertex(const Vec3D& inPos, const Vec3D& inNorm, const Vec2D& inTexCoord) :
			Pos(inPos), Norm(inNorm), TexCoord(inTexCoord) {}
		Vec3D Pos;
		Vec3D Norm;
		Vec2D TexCoord;
	};

	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	ComPtr<ID3D11Buffer> IndexBuffer;
	ComPtr<ID3D11Buffer> VertexBuffer;
};

struct Texture
{
	Texture() = default;
	Texture(Texture&& rhs) noexcept
	{
		ImagePath = std::move(rhs.ImagePath);
		SRV = std::move(rhs.SRV);
	}
	Texture& operator==(Texture&& rhs) noexcept
	{
		if (this != &rhs)
		{
			ImagePath = std::move(rhs.ImagePath);
			SRV = std::move(rhs.SRV);
		}
		return *this;
	}
	Texture(const std::string& inImagePath, ID3D11Device* device, ID3D11DeviceContext* context) : ImagePath(inImagePath)
	{
		Load(inImagePath, device, context);
	}
	void Load(const std::string& inImagePath, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		const Image img(inImagePath);
		ComPtr<ID3D11Texture2D> texture;
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = img.GetWidth();
		texDesc.Height = img.GetHeight();
		texDesc.MipLevels = 0;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HR(device->CreateTexture2D(&texDesc, nullptr, texture.ReleaseAndGetAddressOf()))

			context->UpdateSubresource(texture.Get(), 0,
				nullptr, img.GetBytes(), img.GetWidth() * sizeof(uint8_t) * img.GetChannels(), 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		HR(device->CreateShaderResourceView(texture.Get(), &srvDesc, SRV.ReleaseAndGetAddressOf()))
			context->GenerateMips(SRV.Get());
	}
	std::string ImagePath;
	ComPtr<ID3D11ShaderResourceView> SRV;
};

enum class HLSLType
{
	FLOAT = sizeof(float),
	FLOAT2 = sizeof(float) * 2,
	FLOAT3 = sizeof(float) * 3,
	FLOAT4 = sizeof(float) * 4,
	FLOAT3X3 = sizeof(float) * 3 * 3,
	FLOAT4X4 = sizeof(float) * 4 * 4,
};

class CBuffer
{
public:
	void CreateBuffer(ID3D11Device* device)
	{
		UtilsCreateConstantBuffer(device, Bytes.size(), Buffer.ReleaseAndGetAddressOf());
	}
	void UpdateBuffer(ID3D11DeviceContext* context)
	{
		UtilsUpdateConstantBuffer(context, Bytes.size(), &Bytes[0], Buffer.Get());
	}
	template <typename T>
	void Add(const T& value, const std::string& name)
	{
		size_t offset = 0;
		for (const Entry& e : Entries)
		{
			offset += static_cast<size_t>(e.Type);
		}
		assert(offset == Bytes.size() && "Bytes do no match Etnries");
		Entries.emplace_back(static_cast<HLSLType>(sizeof(T)), name);
		const uint8_t* pValue = reinterpret_cast<const uint8_t*>(&value);
		for (size_t i = 0; i < sizeof(T); ++i)
		{
			Bytes.push_back(pValue[i]);
		}
	}

	template <typename T>
	T* Get(const std::string& name)
	{
		size_t offset = 0;
		for (const Entry& e : Entries)
		{
			offset += static_cast<size_t>(e.Type);
			if (e.Name == name)
			{
				return reinterpret_cast<T>(&Bytes[0] + offset);
			}
		}
		return nullptr;
	}

	template <typename T>
	void Set(const std::string& name, const T& value)
	{
		size_t offset = 0;
		for (const Entry& e : Entries)
		{
			if (e.Name == name)
			{
				T* pData = reinterpret_cast<T*>(&Bytes[0] + offset);
				*pData = value;
				return;
			}
			offset += static_cast<size_t>(e.Type);
		}
		UtilsDebugPrint("WARN: %s not found in constant buffer!\n", name.c_str());
	}

	ID3D11Buffer* GetBuffer() const
	{
		return Buffer.Get();
	}
private:
	struct Entry
	{
		Entry() = default;
		Entry(HLSLType inType, const std::string& inName) : Type(inType), Name(inName)
		{
		}
		HLSLType Type;
		std::string Name;
	};

	ComPtr<ID3D11Buffer> Buffer;
	std::vector<uint8_t> Bytes;
	std::vector<Entry> Entries;
};

namespace Globals
{
	ComPtr<ID3D11SamplerState> DefaultSampler;
	Texture DefaultTexture;
	std::vector<Drawable> Drawables;
	CBuffer PerObjectCB;
	CBuffer PerFrameCB;
};

FogGame::FogGame(): Game::Game()
{
    m_deviceResources = std::make_unique<DeviceResources>();
}

FogGame::~FogGame()
{
}

void FogGame::Tick()
{
	TimerTick(&m_timer);
	Update();
	Render();
}

void FogGame::Initialize(HWND hWnd, uint32_t width, uint32_t height)
{
    Game::Initialize(hWnd, width, height);

    m_shaderManager.Initialize(m_deviceResources->GetDevice(), SHADERS_ROOT, UtilsFormatStr("%s/shader", SRC_ROOT));

    const std::string cubePath = UtilsFormatStr("%s/meshes/cube.obj", ASSETS_ROOT);
	UtilsDebugPrint("Loading %s\n", cubePath.c_str());
	std::unique_ptr<Model> cube = OLLoad(cubePath.c_str());
	Globals::Drawables.emplace_back(cube.get(), m_deviceResources->GetDevice());
	const std::string texPath = UtilsFormatStr("%s/textures/bumpy_bricks_diffuseOriginal.jpg", ASSETS_ROOT);
	Globals::DefaultTexture.Load(texPath, m_deviceResources->GetDevice(), m_deviceResources->GetDeviceContext());

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
		HR(m_deviceResources->GetDevice()->CreateSamplerState(&samplerDesc, Globals::DefaultSampler.ReleaseAndGetAddressOf()))
    }

    {
		Globals::PerObjectCB.Add(MathMat4X4Identity(), "World");
		Globals::PerObjectCB.CreateBuffer(m_deviceResources->GetDevice());
		Globals::PerObjectCB.UpdateBuffer(m_deviceResources->GetDeviceContext());
    }

    {
		Globals::PerFrameCB.Add(MathMat4X4Identity(), "View");
		Globals::PerFrameCB.Add(MathMat4X4Identity(), "Proj");
		Globals::PerFrameCB.Add(Vec3D( 0.0f, 0.0f, -5.0f ), "CameraPos");
		Globals::PerFrameCB.Add(0.0f, "Pad");
		Globals::PerFrameCB.CreateBuffer(m_deviceResources->GetDevice());
		Globals::PerFrameCB.UpdateBuffer(m_deviceResources->GetDeviceContext());
    }
}

void FogGame::OnWindowSizeChanged(int width, int height)
{
    Game::OnWindowSizeChanged(width, height);
}

void FogGame::Update()
{
	m_camera.ProcessMouse(m_timer.DeltaMillis);
	m_camera.ProcessKeyboard(m_timer.DeltaMillis);
	Globals::PerFrameCB.Set("View", m_camera.GetViewMat());
	Globals::PerFrameCB.Set("Proj", m_camera.GetProjMat());
	Globals::PerFrameCB.Set("CameraPos", m_camera.GetPos());
	Globals::PerFrameCB.UpdateBuffer(m_deviceResources->GetDeviceContext());
}

void FogGame::Render()
{

#if WITH_IMGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	UpdateImgui();
#endif

	m_renderer.Clear();
	m_renderer.SetBlendState(nullptr);
	m_renderer.SetDepthStencilState(nullptr);
	m_renderer.SetPrimitiveTopology(R_DEFAULT_PRIMTIVE_TOPOLOGY);
	m_renderer.SetRasterizerState(m_deviceResources->GetRasterizerState());
	m_renderer.BindVertexShader(m_shaderManager.GetVertexShader("VS"));
	m_renderer.BindPixelShader(m_shaderManager.GetPixelShader("PS"));
	m_renderer.SetSamplerState(Globals::DefaultSampler.Get(), 0);
	m_renderer.SetVertexBuffer(Globals::Drawables[0].VertexBuffer.Get(), m_shaderManager.GetStrides(), 0);
	m_renderer.SetIndexBuffer(Globals::Drawables[0].IndexBuffer.Get(), 0);
	m_renderer.SetInputLayout(m_shaderManager.GetInputLayout());
	m_renderer.BindConstantBuffer(BindTargets::VertexShader, Globals::PerObjectCB.GetBuffer(), 0);
	m_renderer.BindConstantBuffer(BindTargets::VertexShader, Globals::PerFrameCB.GetBuffer(), 1);
	m_renderer.BindShaderResource(BindTargets::PixelShader, Globals::DefaultTexture.SRV.Get(), 0);
	m_renderer.DrawIndexed(Globals::Drawables[0].Indices.size(), 0, 0);

#if WITH_IMGUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	m_renderer.Present();
}

void FogGame::CreateWindowSizeDependentResources()
{
	Game::CreateWindowSizeDependentResources();
}

#if WITH_IMGUI
void FogGame::UpdateImgui()
{
	if(ImGui::Button("Recompile shaders"))
	{
		m_shaderManager.Recompile(m_deviceResources->GetDevice());
	}
}
#endif
