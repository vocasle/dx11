#pragma once

#include <d3d11.h>

#include "Utils.h"

namespace D3DHelper
{
	inline void CreateConstantBuffer(ID3D11Device* device,
		size_t byteWidth,
		ID3D11Buffer** pDest)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.ByteWidth = byteWidth;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		if (FAILED(device->CreateBuffer(&bufferDesc, NULL, pDest)))
		{
			UtilsFatalError("ERROR: Failed to create per frame constants cbuffer\n");
		}
	}

	inline void UpdateConstantBuffer(ID3D11DeviceContext* context,
		size_t bufferSize,
		void* data,
		ID3D11Buffer* dest)
	{
		D3D11_MAPPED_SUBRESOURCE mapped = {};

		if (FAILED(context->Map((ID3D11Resource*)dest,
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&mapped)))
		{
			UtilsFatalError("ERROR: Failed to map constant buffer\n");
		}
		memcpy(mapped.pData, data, bufferSize);
		context->Unmap((ID3D11Resource*)dest, 0);
	}
};