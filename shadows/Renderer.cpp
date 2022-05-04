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
	m_DR = dr;
}

void Renderer::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_Topology = topology;
}

void Renderer::SetInputLayout(ID3D11InputLayout* inputLayout)
{
	m_InputLayout = inputLayout;
}

void Renderer::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
	m_RasterizerState = rasterizerState;
}

void Renderer::BindPixelShader(ID3D11PixelShader* shader)
{
	m_PS = shader;
}

void Renderer::BindVertexShader(ID3D11VertexShader* shader)
{
	m_VS = shader;
}

void Renderer::SetSamplerState(ID3D11SamplerState* state, uint32_t slot)
{
	assert(slot < R_MAX_SAMPLERS);
	m_SamplerStates[slot] =state;
}

void Renderer::BindShaderResources(enum BindTargets bindTarget, ID3D11ShaderResourceView** SRVs, uint32_t numSRVs)
{
	assert(numSRVs <= R_MAX_SRV_NUM && "numSRVs is above limit!");

	for (uint32_t i = 0; i < numSRVs; ++i)
	{
		m_PS_SRV[i] = SRVs[i];
	}
}

void Renderer::BindConstantBuffers(enum BindTargets bindTarget, ID3D11Buffer** CBs, uint32_t numCBs)
{
	assert(numCBs <= R_MAX_CB_NUM && "numCBs is above limit!");

	ID3D11Buffer** buffers = bindTarget == BindTargets::PixelShader ? m_PS_CB : m_VS_CB;

	for (uint32_t i = 0; i < numCBs; ++i)
	{
		buffers[i] = CBs[i];
	}
}

void Renderer::BindShaderResource(enum BindTargets bindTarget, ID3D11ShaderResourceView* srv, uint32_t slot)
{
	assert(slot < R_MAX_SRV_NUM);
	m_PS_SRV[slot] = srv;
}

void Renderer::BindConstantBuffer(enum BindTargets bindTarget, ID3D11Buffer* cb, uint32_t slot)
{
	assert(slot < R_MAX_CB_NUM);
	ID3D11Buffer** buffers = bindTarget == BindTargets::PixelShader ? m_PS_CB : m_VS_CB;
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
	ID3D11DeviceContext* context = m_DR->GetDeviceContext();

	context->IASetPrimitiveTopology(m_Topology);
	context->IASetInputLayout(m_InputLayout);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->RSSetState(m_RasterizerState);
	if (m_PS)
	{
		context->PSSetSamplers(0, R_MAX_SAMPLERS, m_SamplerStates);
		context->PSSetShaderResources(0, R_MAX_SRV_NUM, m_PS_SRV);
		context->PSSetConstantBuffers(0, R_MAX_CB_NUM, m_PS_CB);
	}
	context->VSSetShader(m_VS, NULL, 0);
	context->PSSetShader(m_PS, NULL, 0);
	context->VSSetConstantBuffers(0, R_MAX_CB_NUM, m_VS_CB);
	context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);

}

void Renderer::Clear()
{
	ID3D11DeviceContext* ctx = m_DR->GetDeviceContext();
	ID3D11RenderTargetView* rtv = m_DR->GetRenderTargetView();
	ID3D11DepthStencilView* dsv = m_DR->GetDepthStencilView();

	static const float CLEAR_COLOR[4] = { 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };
	static const float BLACK_COLOR[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	ctx->ClearRenderTargetView(rtv, BLACK_COLOR);
	ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ctx->OMSetRenderTargets(1, &rtv, dsv);
	ctx->RSSetViewports(1, &m_DR->GetViewport());
}

void Renderer::Present()
{
	const HRESULT hr = m_DR->GetSwapChain()->Present(1, 0);

	ID3D11DeviceContext1* ctx = reinterpret_cast<ID3D11DeviceContext1*>(m_DR->GetDeviceContext());
	ctx->DiscardView((ID3D11View*)m_DR->GetRenderTargetView());
	if (m_DR->GetDepthStencilView())
	{
		ctx->DiscardView((ID3D11View*)m_DR->GetDepthStencilView());
	}

	if (FAILED(hr))
	{
		OutputDebugStringA("ERROR: Failed to present\n");
		ExitProcess(EXIT_FAILURE);
	}
}
