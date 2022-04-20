#include "ShadowMap.h"
#include "Utils.h"

ShadowMap::ShadowMap(): m_OutputViewPort{}
{
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::InitResources(ID3D11Device* device, uint32_t texWidth, uint32_t texHeight)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* depthTex;
	HR(device->CreateTexture2D( &texDesc, NULL, &depthTex))

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	HR(device->CreateDepthStencilView( depthTex, &dsvDesc,
		m_pOutputTextureDSV.ReleaseAndGetAddressOf()))

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	HR(device->CreateShaderResourceView( depthTex, &srvDesc,
		m_pOutputTextureSRV.ReleaseAndGetAddressOf()))

	COM_FREE(depthTex);

	m_OutputViewPort.TopLeftX = 0.0f;
	m_OutputViewPort.TopLeftY = 0.0f;
	m_OutputViewPort.Width = (float)texWidth;
	m_OutputViewPort.Height = (float)texHeight;
	m_OutputViewPort.MinDepth = 0.0f;
	m_OutputViewPort.MaxDepth = 1.0f;
}

void ShadowMap::Bind(ID3D11DeviceContext* ctx)
{
	ctx->ClearDepthStencilView(m_pOutputTextureDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	ctx->OMSetRenderTargets(0, 0, m_pOutputTextureDSV.Get());
	ctx->RSSetViewports(1, &m_OutputViewPort);
}