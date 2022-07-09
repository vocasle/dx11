#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>

class Texture
{
public:
	Texture(DXGI_FORMAT format, int width, int height, ID3D11Device* device);
	Texture(const std::string& filepath, ID3D11Device* device);
	void GenerateMips(ID3D11DeviceContext* context);

	ID3D11ShaderResourceView* GetSRV() const { return mSRV.Get(); }
	ID3D11RenderTargetView* GetRTV() const { return mRTV.Get(); }

private:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRTV;

	int mWidth;
	int mHeight;
	DXGI_FORMAT mFormat;

};
