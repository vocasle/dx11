#pragma once

#include <wrl/client.h>
#include <d3d11.h>

class CubeMap
{
public:
	CubeMap();
	~CubeMap();

	void LoadCubeMap(ID3D11Device* device, const char* filepath);

private:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;


	ComPtr<ID3D11ShaderResourceView> m_cubeMap;
};
