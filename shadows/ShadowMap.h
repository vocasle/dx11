#pragma once

#include <d3d11.h>
#include <cstdint>
#include <wrl/client.h>

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	void InitResources(ID3D11Device* device, uint32_t texWidth, uint32_t texHeight);

	void Bind(ID3D11DeviceContext* ctx);

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pOutputTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_pOutputTextureDSV;
	D3D11_VIEWPORT										m_OutputViewPort;
};