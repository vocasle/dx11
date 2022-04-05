#include "Shader.h"

#include "Utils.h"

#include <cstdint>
#include <exception>
#include <vector>

Shader::Shader(const char* filepath, ID3D11Device* device, ShaderType type):
	m_shader(nullptr),
	m_type(type)
{
	std::vector<uint8_t> shaderBytecode = UtilsReadData(filepath);

	HRESULT hr = 0;

	switch (type)
	{
	case ShaderType::Vertex:
		hr = device->CreateVertexShader(&shaderBytecode[0], shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11VertexShader**>(&m_shader));
		break;
	case ShaderType::Pixel:
		hr = device->CreatePixelShader(&shaderBytecode[0], shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11PixelShader**>(&m_shader));
		break;
	case ShaderType::Geometry:
		hr = device->CreateGeometryShader(&shaderBytecode[0], shaderBytecode.size(), nullptr, reinterpret_cast<ID3D11GeometryShader**>(&m_shader));
		break;
	default:
		UTILS_FATAL_ERROR("Failed to create shader. Unknown shader type.");
		break;
	}

	if (FAILED(hr))
	{
		UTILS_FATAL_ERROR("Failed to create shader %s", filepath);
	}
}

Shader::~Shader()
{
	COM_FREE(m_shader);
}
