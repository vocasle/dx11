#pragma once

#include <d3d11.h>
#include <stdint.h>

typedef struct ShadowMap
{
	ID3D11ShaderResourceView*	m_pOutputTextureSRV;
	ID3D11RenderTargetView*		m_pOutputTextureRTV;
	ID3D11DepthStencilView*		m_pOutputTextureDSV;
	D3D11_VIEWPORT				m_OutputViewPort;
} ShadowMap;

ShadowMap* SMNew(void);
void SMFree(ShadowMap* sm);

void SMInit(ShadowMap* sm);
void SMDeinit(ShadowMap* sm);

void SMInitResources(ShadowMap* sm, ID3D11Device* device, uint32_t texWidth, uint32_t texHeight);

void SMBind(ShadowMap* sm, ID3D11DeviceContext* ctx);