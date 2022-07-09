#include "Texture.h"

#include "Utils.h"

Texture::Texture(DXGI_FORMAT format, int width, int height, ID3D11Device* device): mWidth(width), mHeight(height), mFormat(format)
{

	CD3D11_TEXTURE2D_DESC desc(format, width, height, 1, 0,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, 0, 1, 0);

	HR(device->CreateTexture2D(&desc, nullptr, mTexture.ReleaseAndGetAddressOf()))

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	HR(device->CreateShaderResourceView(mTexture.Get(), &srvDesc, mSRV.ReleaseAndGetAddressOf()))

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	HR(device->CreateRenderTargetView(mTexture.Get(), &rtvDesc, mRTV.ReleaseAndGetAddressOf()))
}


void Texture::GenerateMips(ID3D11DeviceContext* context)
{
//	context->UpdateSubresource(mTexture.Get(), 0, nullptr, bytes, width * sizeof(uint8_t) * desiredChannels, 0);
}
