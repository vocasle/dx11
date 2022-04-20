#include "Renderer.h"

#include <cassert>


Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::SetDeviceResources(DeviceResources* dr)
{
	DR = dr;
}

void Renderer::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	Topology = topology;
}

void Renderer::SetInputLayout(ID3D11InputLayout* inputLayout)
{
	InputLayout = inputLayout;
}

void Renderer::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
	RasterizerState = rasterizerState;
}

void Renderer::BindPixelShader(ID3D11PixelShader* shader)
{
	PS = shader;
}

void Renderer::BindVertexShader(ID3D11VertexShader* shader)
{
	VS = shader;
}

void Renderer::SetSamplerState(ID3D11SamplerState* state)
{
	SamplerState = state;
}

void Renderer::BindShaderResources(enum BindTargets bindTarget, ID3D11ShaderResourceView** SRVs, uint32_t numSRVs)
{
	assert(numSRVs <= R_MAX_SRV_NUM && "numSRVs is above limit!");

	for (uint32_t i = 0; i < numSRVs; ++i)
	{
		PS_SRV[i] = SRVs[i];
	}
}

void Renderer::BindConstantBuffers(enum BindTargets bindTarget, ID3D11Buffer** CBs, uint32_t numCBs)
{
	assert(numCBs <= R_MAX_CB_NUM && "numCBs is above limit!");

	ID3D11Buffer** buffers = bindTarget == BindTargets::PixelShader ? PS_CB : VS_CB;

	for (uint32_t i = 0; i < numCBs; ++i)
	{
		buffers[i] = CBs[i];
	}
}

void Renderer::BindShaderResource(enum BindTargets bindTarget, ID3D11ShaderResourceView* srv, uint32_t slot)
{
	assert(slot < R_MAX_SRV_NUM);
	PS_SRV[slot] = srv;
}

void Renderer::BindConstantBuffer(enum BindTargets bindTarget, ID3D11Buffer* cb, uint32_t slot)
{
	assert(slot < R_MAX_CB_NUM);
	ID3D11Buffer** buffers = bindTarget == BindTargets::PixelShader ? PS_CB : VS_CB;
	buffers[slot] = cb;
}

void Renderer::DrawIndexed(ID3D11Buffer* indexBuffer,
	ID3D11Buffer* vertexBuffer,
	uint32_t strides,
	uint32_t indexCount,
	uint32_t startIndexLocation,
	uint32_t baseVertexLocation)
{
	const uint32_t offsets = 0;
	static ID3D11ShaderResourceView* nullSRV[] = { NULL };
	ID3D11DeviceContext* context = DR->GetDeviceContext();

	context->IASetPrimitiveTopology(Topology);
	context->IASetInputLayout(InputLayout);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->RSSetState(RasterizerState);
	context->PSSetSamplers(0, 1, &SamplerState);
	context->VSSetShader(VS, NULL, 0);
	context->PSSetShader(PS, NULL, 0);

	context->PSSetShaderResources(0, R_MAX_SRV_NUM, PS_SRV);
	context->PSSetConstantBuffers(0, R_MAX_CB_NUM, PS_CB);
	context->VSSetConstantBuffers(0, R_MAX_CB_NUM, VS_CB);
	context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);

}

void Renderer::Clear()
{
	ID3D11DeviceContext* ctx = DR->GetDeviceContext();
	ID3D11RenderTargetView* rtv = DR->GetRenderTargetView();
	ID3D11DepthStencilView* dsv = DR->GetDepthStencilView();

	static const float CLEAR_COLOR[4] = { 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };
	static const float BLACK_COLOR[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ctx->ClearRenderTargetView(rtv, BLACK_COLOR);
	ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ctx->OMSetRenderTargets(1, &rtv, dsv);
	ctx->RSSetViewports(1, &DR->GetViewport());
}

void Renderer::Present()
{
	const HRESULT hr = DR->GetSwapChain()->Present(1, 0);

	ID3D11DeviceContext1* ctx = reinterpret_cast<ID3D11DeviceContext1*>(DR->GetDeviceContext());
	ctx->DiscardView((ID3D11View*)DR->GetRenderTargetView());
	if (DR->GetDepthStencilView())
	{
		ctx->DiscardView((ID3D11View*)DR->GetDepthStencilView());
	}

	if (FAILED(hr))
	{
		OutputDebugStringA("ERROR: Failed to present\n");
		ExitProcess(EXIT_FAILURE);
	}
}
