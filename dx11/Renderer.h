#pragma once

#include <stdint.h>

#include <d3d11.h>

#include "DeviceResources.h"

#define R_MAX_SRV_NUM 4
#define R_MAX_CB_NUM 2

#define R_DEFAULT_PRIMTIVE_TOPOLOGY D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST

struct Renderer
{
	D3D11_PRIMITIVE_TOPOLOGY Topology;
	ID3D11InputLayout* InputLayout;
	ID3D11RasterizerState* RasterizerState;
	ID3D11SamplerState* SamplerState;
	ID3D11PixelShader* PS;
	ID3D11VertexShader* VS;
	ID3D11ShaderResourceView* PS_SRV[R_MAX_SRV_NUM];
	uint32_t NumPS_SRV;
	ID3D11Buffer* PS_CB[R_MAX_CB_NUM];
	uint32_t NumPS_CB;
	ID3D11Buffer* VS_CB[R_MAX_CB_NUM];
	uint32_t NumVS_CB;
	DeviceResources* DR;
};

enum BindTargets
{
	BindTargets_PS,
	BindTargets_VS
};

void RInit(struct Renderer* renderer);

void RDeinit(struct Renderer* renderer);

void RSetDeviceResources(struct Renderer* renderer, DeviceResources* dr);

void RSetPrimitiveTopology(struct Renderer* renderer, D3D11_PRIMITIVE_TOPOLOGY topology);

void RSetInputLayout(struct Renderer* renderer, ID3D11InputLayout* inputLayout);

void RSetRasterizerState(struct Renderer* renderer, ID3D11RasterizerState* rasterizerState);

void RSetSamplerState(struct Renderer* renderer, ID3D11SamplerState* state);

void RBindPixelShader(struct Renderer* renderer, ID3D11PixelShader* shader);

void RBindVertexShader(struct Renderer* renderer, ID3D11VertexShader* shader);

void RBindShaderResources(struct Renderer* renderer, enum BindTargets bindTarget, ID3D11ShaderResourceView** SRVs, uint32_t numSRVs);

void RBindConstantBuffers(struct Renderer* renderer, enum BindTargets bindTarget, ID3D11Buffer** CBs, uint32_t numCBs);

void RDrawIndexed(struct Renderer* renderer,
	ID3D11Buffer* indexBuffer, 
	ID3D11Buffer* vertexBuffer,
	uint32_t strides,
	uint32_t indexCount,
	uint32_t startIndexLocation, 
	uint32_t baseVertexLocation);

void RClear(struct Renderer* renderer);

void RPresent(struct Renderer* renderer);
