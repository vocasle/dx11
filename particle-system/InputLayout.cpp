#include "InputLayout.h"
#include "Utils.h"

InputLayout::InputLayout()
{
}

void InputLayout::CreateDefaultLayout(ID3D11Device* device, unsigned char* bytes, size_t bufferSize)
{
	const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"NORMAL",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				12,
				D3D11_INPUT_PER_VERTEX_DATA,
				0,
			},
			{
				"TANGENT",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				24,
				D3D11_INPUT_PER_VERTEX_DATA,
				0,
			},
			{
				"BITANGENT",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				36,
				D3D11_INPUT_PER_VERTEX_DATA,
				0,
			},
			{
				"TEXCOORDS",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				48,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
	};
	HR(device->CreateInputLayout(inputElementDesc, 5, bytes, bufferSize, m_defaultLayout.ReleaseAndGetAddressOf()))
}

void InputLayout::CreateSkyLayout(ID3D11Device* device, unsigned char* bytes, size_t bufferSize)
{
	const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
	};
	HR(device->CreateInputLayout(inputElementDesc, 1, bytes, bufferSize, m_skyLayout.ReleaseAndGetAddressOf()))
}

size_t InputLayout::GetVertexSize(VertexType vertexType) const
{
	return static_cast<size_t>(vertexType);
}