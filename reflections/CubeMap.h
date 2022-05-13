#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

#include "Math.h"
#include "Actor.h"
#include "Camera.h"

class CubeMap
{
public:
	CubeMap();
	~CubeMap();

	void LoadCubeMap(ID3D11Device* device, const std::vector<const char*>& filepaths);
	ID3D11ShaderResourceView* GetCubeMap() const { return m_cubeMap.Get(); }
	ID3D11SamplerState* GetCubeMapSampler() const { return m_sampler.Get(); }
	void SetActor(const Actor& actor);
	void Draw(ID3D11DeviceContext* ctx);
	void SetCamera(Camera* camera);

private:
	void CreateSampler(ID3D11Device* device);

	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D11ShaderResourceView> m_cubeMap;
	ComPtr<ID3D11SamplerState> m_sampler;

	Actor m_cube;
	Camera* m_camera;
};
