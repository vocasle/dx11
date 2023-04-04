#include "Renderer.h"

#include <assert.h>

void RInit(struct Renderer* renderer)
{
	memset(renderer, 0, sizeof(struct Renderer));
}

void RDeinit(struct Renderer* renderer)
{
	renderer->InputLayout = NULL;
	renderer->RasterizerState = NULL;
	renderer->SamplerState = NULL;
	renderer->PS = NULL;
	renderer->VS = NULL;
	for (uint32_t i = 0; i < R_MAX_SRV_NUM; ++i)
	{
		renderer->PS_SRV[i] = NULL;
	}

	for (uint32_t i = 0; i < R_MAX_CB_NUM; ++i)
	{
		renderer->PS_CB[i] = NULL;
		renderer->VS_CB[i] = NULL;
	}
}

void RSetDeviceResources(struct Renderer* renderer, DeviceResources* dr)
{
	renderer->DR = dr;
}

void RSetPrimitiveTopology(struct Renderer* renderer, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	renderer->Topology = topology;
}

void RSetInputLayout(struct Renderer* renderer, ID3D11InputLayout* inputLayout)
{
	renderer->InputLayout = inputLayout;
}

void RSetRasterizerState(struct Renderer* renderer, ID3D11RasterizerState* rasterizerState)
{
	renderer->RasterizerState = rasterizerState;
}

void RBindPixelShader(struct Renderer* renderer, ID3D11PixelShader* shader)
{
	renderer->PS = shader;
}

void RBindVertexShader(struct Renderer* renderer, ID3D11VertexShader* shader)
{
	renderer->VS = shader;
}

void RSetSamplerState(struct Renderer* renderer, ID3D11SamplerState* state)
{
	renderer->SamplerState = state;
}

void RBindShaderResources(struct Renderer* renderer, enum BindTargets bindTarget, ID3D11ShaderResourceView** SRVs, uint32_t numSRVs)
{
	assert(numSRVs <= R_MAX_SRV_NUM && "numSRVs is above limit!");

	for (uint32_t i = 0; i < numSRVs; ++i)
	{
		renderer->PS_SRV[i] = SRVs[i];
	}
	renderer->NumPS_SRV = numSRVs;
}

void RBindConstantBuffers(struct Renderer* renderer, enum BindTargets bindTarget, ID3D11Buffer** CBs, uint32_t numCBs)
{
	assert(numCBs <= R_MAX_CB_NUM && "numCBs is above limit!");

	ID3D11Buffer** buffers = bindTarget == BindTargets_PS ? renderer->PS_CB : renderer->VS_CB;
	uint32_t* numBuffers = bindTarget == BindTargets_PS ? &renderer->NumPS_CB : &renderer->NumVS_CB;

	for (uint32_t i = 0; i < numCBs; ++i)
	{
		buffers[i] = CBs[i];
	}
	*numBuffers = numCBs;
}

void RDrawIndexed(struct Renderer* renderer,
	ID3D11Buffer* indexBuffer,
	ID3D11Buffer* vertexBuffer,
	uint32_t strides,
	uint32_t indexCount,
	uint32_t startIndexLocation,
	uint32_t baseVertexLocation)
{
	const uint32_t offsets = 0;
	static ID3D11ShaderResourceView* nullSRV[] = { NULL };
	ID3D11DeviceContext* context = renderer->DR->Context;

	context->lpVtbl->IASetPrimitiveTopology(context, renderer->Topology);
	context->lpVtbl->IASetInputLayout(context, renderer->InputLayout);
	context->lpVtbl->IASetVertexBuffers(context, 0, 1, &vertexBuffer, &strides, &offsets);
	context->lpVtbl->IASetIndexBuffer(context, indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->lpVtbl->RSSetState(context, renderer->RasterizerState);
	context->lpVtbl->PSSetSamplers(context, 0, 1, &renderer->SamplerState);
	context->lpVtbl->VSSetShader(context, renderer->VS, NULL, 0);
	context->lpVtbl->PSSetShader(context, renderer->PS, NULL, 0);

	if (renderer->NumPS_SRV > 0)
	{
		context->lpVtbl->PSSetShaderResources(context, 0, renderer->NumPS_SRV, renderer->PS_SRV);
	}
	else
	{
		context->lpVtbl->PSSetShaderResources(context, 0, 1, nullSRV);
	}

	if (renderer->NumPS_CB > 0)
	{
		context->lpVtbl->PSSetConstantBuffers(context, 0, renderer->NumPS_CB, renderer->PS_CB);
	}

	if (renderer->NumVS_CB > 0)
	{
		context->lpVtbl->VSSetConstantBuffers(context, 0, renderer->NumVS_CB, renderer->VS_CB);
	}

	context->lpVtbl->DrawIndexed(context, indexCount, startIndexLocation, baseVertexLocation);
}

void RClear(struct Renderer* renderer)
{
	ID3D11DeviceContext1* ctx = renderer->DR->Context;
	ID3D11RenderTargetView* rtv = renderer->DR->RenderTargetView;
	ID3D11DepthStencilView* dsv = renderer->DR->DepthStencilView;

	static const float CLEAR_COLOR[4] = { 0, 0, 0, 1 };

	ctx->lpVtbl->Flush(ctx);

	ctx->lpVtbl->ClearRenderTargetView(ctx, rtv, CLEAR_COLOR);
	ctx->lpVtbl->ClearDepthStencilView(ctx, dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ctx->lpVtbl->OMSetRenderTargets(ctx, 1, &rtv, dsv);
	ctx->lpVtbl->RSSetViewports(ctx, 1, &renderer->DR->ScreenViewport);
}

void RPresent(struct Renderer* renderer)
{
	const HRESULT hr = renderer->DR->SwapChain->lpVtbl->Present(renderer->DR->SwapChain, 1, 0);

	ID3D11DeviceContext1* ctx = renderer->DR->Context;
	ctx->lpVtbl->DiscardView(ctx, (ID3D11View*)renderer->DR->RenderTargetView);
	if (renderer->DR->DepthStencilView)
	{
		ctx->lpVtbl->DiscardView(ctx, (ID3D11View*)renderer->DR->DepthStencilView);
	}

	if (FAILED(hr))
	{
		OutputDebugStringA("ERROR: Failed to present\n");
		ExitProcess(EXIT_FAILURE);
	}
}
