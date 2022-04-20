#pragma once

#include <stdint.h>

#include <d3d11.h>

#include "DeviceResources.h"

#define R_MAX_SRV_NUM 4
#define R_MAX_CB_NUM 3

#define R_DEFAULT_PRIMTIVE_TOPOLOGY D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST

enum class BindTargets
{
	PixelShader,
	VertexShader
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void SetDeviceResources(DeviceResources* dr);
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	void SetInputLayout(ID3D11InputLayout* inputLayout);
	void SetRasterizerState(ID3D11RasterizerState* rasterizerState);
	void SetSamplerState(ID3D11SamplerState* state);
	
	void BindPixelShader(ID3D11PixelShader* shader);
	void BindVertexShader(ID3D11VertexShader* shader);
	void BindShaderResources(enum BindTargets bindTarget, ID3D11ShaderResourceView** SRVs, uint32_t numSRVs);
	void BindConstantBuffers(enum BindTargets bindTarget, ID3D11Buffer** CBs, uint32_t numCBs);
	void BindShaderResource(enum BindTargets bindTarget, ID3D11ShaderResourceView* srv, uint32_t slot);
	void BindConstantBuffer(enum BindTargets bindTarget, ID3D11Buffer* cb, uint32_t slot);
	
	void DrawIndexed(ID3D11Buffer* indexBuffer,
		ID3D11Buffer* vertexBuffer,
		uint32_t strides,
		uint32_t indexCount,
		uint32_t startIndexLocation,
		uint32_t baseVertexLocation);
	void Clear();
	void Present();

private:
	D3D11_PRIMITIVE_TOPOLOGY Topology;
	ID3D11InputLayout* InputLayout;
	ID3D11RasterizerState* RasterizerState;
	ID3D11SamplerState* SamplerState;
	ID3D11PixelShader* PS;
	ID3D11VertexShader* VS;
	ID3D11ShaderResourceView* PS_SRV[R_MAX_SRV_NUM];
	ID3D11Buffer* PS_CB[R_MAX_CB_NUM];
	ID3D11Buffer* VS_CB[R_MAX_CB_NUM];
	DeviceResources* DR;
};
