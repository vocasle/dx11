#include "ShadowMap.h"
#include "Utils.h"

ShadowMap* SMNew(void)
{
	ShadowMap* sm = malloc(sizeof(ShadowMap));
	SMInit(sm);
	return sm;
}

void SMFree(ShadowMap* sm)
{
	SMDeinit(sm);
	free(sm);
}

void SMInit(ShadowMap* sm)
{
	memset(sm, 0, sizeof(ShadowMap));
}

void SMDeinit(ShadowMap* sm)
{
	COM_FREE(sm->m_pOutputTextureDSV);
	COM_FREE(sm->m_pOutputTextureSRV);
}

void SMInitResources(ShadowMap* sm, ID3D11Device* device, uint32_t texWidth, uint32_t texHeight)
{
	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* depthTex;
	HR(device->lpVtbl->CreateTexture2D(device, &texDesc, NULL, &depthTex))

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { 0 };
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	HR(device->lpVtbl->CreateDepthStencilView(device, depthTex, &dsvDesc,
		&sm->m_pOutputTextureDSV))

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { 0 };
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	HR(device->lpVtbl->CreateShaderResourceView(device, depthTex, &srvDesc,
		&sm->m_pOutputTextureSRV))

	COM_FREE(depthTex);

	sm->m_OutputViewPort.TopLeftX = 0.0f;
	sm->m_OutputViewPort.TopLeftY = 0.0f;
	sm->m_OutputViewPort.Width = (float)texWidth;
	sm->m_OutputViewPort.Height = (float)texHeight;
	sm->m_OutputViewPort.MinDepth = 0.0f;
	sm->m_OutputViewPort.MaxDepth = 1.0f;
}

void SMBind(ShadowMap* sm, ID3D11DeviceContext* ctx)
{
	ctx->lpVtbl->ClearDepthStencilView(ctx, sm->m_pOutputTextureDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	ctx->lpVtbl->OMSetRenderTargets(ctx, 0, 0, sm->m_pOutputTextureDSV);
	ctx->lpVtbl->RSSetViewports(ctx, 1, &sm->m_OutputViewPort);
}