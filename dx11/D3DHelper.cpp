#include "D3DHelper.h"

#include <DirectXTex.h>
#include <vector>

#include "Utils.h"
#include "stb_image.h"

namespace D3DHelper
{
	static void LoadTextureFromDDSFile(ID3D11Device* device, const char* filename, struct Texture* texture)
	{
		using namespace DirectX;

		TexMetadata info = {};
		ScratchImage image = {};
		if (FAILED(LoadFromDDSFile(UtilsString2WideString(filename).c_str(), DDS_FLAGS_NONE, &info, image)))
		{
			UTILS_FATAL_ERROR("Failed to load texture from %s", filename);
		}

		if (FAILED(CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), info, &texture->SRV)))
		{
			UTILS_FATAL_ERROR("Failed to create shader resource view from %s", filename);
		}

		image.Release();
	}

	void LoadTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const char* filename, struct Texture* texture)
	{
		if (std::string(filename).ends_with(".dds"))
		{
			LoadTextureFromDDSFile(device, filename, texture);
			return;
		}

		int width = 0;
		int height = 0;
		int channelsInFile = 0;
		const int desiredChannels = 4;

		FILE* f = nullptr;
		fopen_s(&f, filename, "r");
		if (f)
		{
			fclose(f);
		}
		else
		{
			printf("Shit");
		}

		unsigned char* bytes = stbi_load(filename, &width, &height, &channelsInFile, desiredChannels);
		if (!bytes)
		{
			UtilsDebugPrint("ERROR: Failed to load texture from %s\n", filename);
			ExitProcess(EXIT_FAILURE);
		}

		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 0;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			desc.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pSysMem = bytes;
			subresourceData.SysMemPitch = width * sizeof(unsigned char) * desiredChannels;


			if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture->Resource)))
			{
				UtilsDebugPrint("ERROR: Failed to create texture from file %s\n", filename);
				ExitProcess(EXIT_FAILURE);
			}
		
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = -1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			if (FAILED(device->CreateShaderResourceView((ID3D11Resource*)texture->Resource, &srvDesc, &texture->SRV)))
			{
				UtilsDebugPrint("ERROR: Failed to create SRV from file %s\n", filename);
				ExitProcess(EXIT_FAILURE);
			}

			context->UpdateSubresource(texture->Resource,
				0,
				nullptr,
				bytes,
				width * sizeof(uint8_t) * desiredChannels,
				width * height);

			context->GenerateMips(texture->SRV);
		}

		stbi_image_free(bytes);
	}

	void UpdateConstantBuffer(ID3D11DeviceContext* context,
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

	void CreateConstantBuffer(ID3D11Device* device,
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

	ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device)
	{
		// 
		// Create the random data.
		//
		std::vector<Vec4D> randomValues(1024);

		for (Vec4D vec : randomValues)
		{
			vec.X = MathRandom(-1.0f, 1.0f);
			vec.Y = MathRandom(-1.0f, 1.0f);
			vec.Z = MathRandom(-1.0f, 1.0f);
			vec.W = MathRandom(-1.0f, 1.0f);
		}

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &randomValues[0];
		initData.SysMemPitch = randomValues.size() * sizeof(Vec4D);
		initData.SysMemSlicePitch = 0;

		//
		// Create the texture.
		//
		D3D11_TEXTURE1D_DESC texDesc = {};
		texDesc.Width = 1024;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		texDesc.ArraySize = 1;

		ID3D11Texture1D* randomTex = 0;
		HR(device->CreateTexture1D(&texDesc, &initData, &randomTex));

		//
		// Create the resource view.
		//
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		viewDesc.Format = texDesc.Format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
		viewDesc.Texture1D.MostDetailedMip = 0;

		ID3D11ShaderResourceView* randomTexSRV = 0;
		HR(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

		COM_FREE(randomTex);

		return randomTexSRV;
	}
};