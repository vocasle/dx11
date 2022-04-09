#pragma once

#include <d3d11.h>

#include "D3DCommonTypes.h"

namespace D3DHelper
{
	void CreateConstantBuffer(ID3D11Device* device,
		size_t byteWidth,
		ID3D11Buffer** pDest);

	void UpdateConstantBuffer(ID3D11DeviceContext* context,
		size_t bufferSize,
		void* data,
		ID3D11Buffer* dest);

	void LoadTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const char* filename, struct Texture* texture);
};