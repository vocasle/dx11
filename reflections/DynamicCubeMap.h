#pragma once

#include "Camera.h"
#include "Math.h"

#include <d3d11.h>
#include <wrl/client.h>

class DynamicCubeMap
{
public:

	void Init(ID3D11Device* device);
	void BuildCubeFaceCamera(const Vec3D& origin);

private:

	void CreateRenderTargets(ID3D11Device* device);
	void CreateDepthBuffer(ID3D11Device* device);
	void CreateViewport(ID3D11Device* device);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_dynamicCubeMapSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dynamicCubeMapDSV;
	D3D11_VIEWPORT m_cubeMapViewport;
	ID3D11RenderTargetView* m_dynamicCubeMapRTV[6];
	Camera m_cubeMapCamera[6];

	static const int CUBEMAP_SIZE = 256;

};
