#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

class ShadowMap {
    public:
	ShadowMap();
	~ShadowMap();

	void InitResources(ID3D11Device *device, uint32_t texWidth,
			   uint32_t texHeight);

	void Bind(ID3D11DeviceContext *ctx);
	void Unbind(ID3D11DeviceContext *ctx);

	ID3D11ShaderResourceView *GetDepthMapSRV() const
	{
		return m_pOutputTextureSRV.Get();
	}
	ID3D11SamplerState *GetShadowSampler() const
	{
		return m_ShadowSampler.Get();
	}

    private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pOutputTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pOutputTextureDSV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_ShadowSampler;
	D3D11_VIEWPORT m_OutputViewPort;
};