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

void CubeMap::LoadCubeMap(ID3D11Device* device, const std::vector<const char*>& filepaths)
{
	int width = 0;
	int height = 0;
	int channelsInFile = 0;
	const int desiredChannels = 4;
	uint8_t* bytes = stbi_load(filepaths[0], &width, &height, &channelsInFile, desiredChannels);
	stbi_image_free(bytes);


	//Description of each face
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc = {};
	SMViewDesc.Format = texDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA pData[6];
	uint8_t* pBytes[6];

	for (uint32_t i = 0; i < filepaths.size(); ++i)
	{
		pBytes[i] = stbi_load(filepaths[i], &width, &height, &channelsInFile, desiredChannels);
		assert(pBytes[i] && "stbi_load failed");
		pData[i].pSysMem = pBytes[i];
		pData[i].SysMemPitch = width * desiredChannels;
		
		if (!pBytes[i])
		{
			UtilsDebugPrint("ERROR: Failed to load texture from %s\n", filepaths[i]);
			ExitProcess(EXIT_FAILURE);
		}
	}
	
	ComPtr<ID3D11Texture2D> texture;
	HR(device->CreateTexture2D(&texDesc, &pData[0], texture.ReleaseAndGetAddressOf()));
	HR(device->CreateShaderResourceView(texture.Get(), &SMViewDesc, m_cubeMap.ReleaseAndGetAddressOf()));

	for (uint32_t i = 0; i < 6; ++i)
	{
		stbi_image_free(pBytes[i]);
	}

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