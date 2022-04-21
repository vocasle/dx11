#include "CubeMap.h"
#include "stb_image.h"
#include "Utils.h"

using namespace Microsoft::WRL;

CubeMap::CubeMap()
{
}

CubeMap::~CubeMap()
{
}

void CubeMap::LoadCubeMap(ID3D11Device* device, const char* filepath)
{
	int width = 0;
	int height = 0;
	int channelsInFile = 0;
	const int desiredChannels = 4;

	unsigned char* bytes = stbi_load(filepath, &width, &height, &channelsInFile, desiredChannels);
	if (!bytes)
	{
		UtilsDebugPrint("ERROR: Failed to load texture from %s\n", filepath);
		ExitProcess(EXIT_FAILURE);
	}
	ComPtr<ID3D11Texture2D> texture;
	{
		D3D11_TEXTURE2D_DESC desc = {};
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

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = bytes;
		subresourceData.SysMemPitch = width * sizeof(unsigned char) * desiredChannels;

		HR(device->CreateTexture2D(&desc, &subresourceData, texture.ReleaseAndGetAddressOf()))
	}

	{
		// TODO: Fix mip map generation
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		memset(&srvDesc, 0, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;

		HR(device->CreateShaderResourceView(texture.Get(), &srvDesc, m_cubeMap.ReleaseAndGetAddressOf()));
	}

	stbi_image_free(bytes);

	CreateSampler(device);
}

void CubeMap::SetActor(const Actor& actor)
{
	m_cube = actor;
}

void CubeMap::Draw(ID3D11DeviceContext* ctx)
{
	uint32_t strides =  sizeof(Vertex);
	uint32_t offsets = 0;

	ID3D11Buffer* vb = m_cube.GetVertexBuffer();
	ctx->IASetVertexBuffers(0, 1, &vb, &strides, &offsets);
	ctx->IASetIndexBuffer(m_cube.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	// Erase the translation component to avoid skybox jitter caused by camera movement
	Mat4X4 V = m_camera->GetViewMat();
	V.V[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//skyEffect.SetWorldMatrix(XMMatrixIdentity());
	//skyEffect.SetViewMatrix(V);
	//skyEffect.SetProjMatrix(camera.GetProjXM());
	//skyEffect.SetTextureCube(m_pTextureCubeSRV.Get());
	//skyEffect.Apply(deviceContext);
	ctx->DrawIndexed(m_cube.GetNumIndices(), 0, 0);
}

void CubeMap::SetCamera(Camera* camera)
{
	m_camera = camera;
}

void CubeMap::CreateSampler(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC desc = {};
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	
	HR(device->CreateSamplerState(&desc, m_sampler.ReleaseAndGetAddressOf()));
}