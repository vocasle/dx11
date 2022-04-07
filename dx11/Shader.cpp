#include "Shader.h"

#include "Utils.h"

#include <cstdint>
#include <exception>
#include <vector>

Shader::Shader():
	m_bytecode{},
	m_shader{nullptr},
	m_type{ShaderType::None}
{
}

Shader::Shader(const char* filepath, ID3D11Device* device, ShaderType type):
	m_shader(nullptr),
	m_type(type),
	m_bytecode()
{
	m_bytecode = UtilsReadData(filepath);

	HRESULT hr = 0;

	switch (type)
	{
	case ShaderType::Vertex:
		hr = device->CreateVertexShader(&m_bytecode[0], m_bytecode.size(), nullptr, reinterpret_cast<ID3D11VertexShader**>(&m_shader));
		break;
	case ShaderType::Pixel:
		hr = device->CreatePixelShader(&m_bytecode[0], m_bytecode.size(), nullptr, reinterpret_cast<ID3D11PixelShader**>(&m_shader));
		break;
	case ShaderType::Geometry:
		hr = device->CreateGeometryShader(&m_bytecode[0], m_bytecode.size(), nullptr, reinterpret_cast<ID3D11GeometryShader**>(&m_shader));
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

Shader::Shader(const Shader& shader)
{
	m_bytecode = shader.m_bytecode;
	m_shader = shader.m_shader;
	m_type = shader.m_type;
}

Shader& Shader::operator=(const Shader& shader)
{
	if (this != &shader)
	{
		COM_FREE(m_shader);
		m_type = shader.m_type;
		m_bytecode = shader.m_bytecode;
		m_shader = shader.m_shader;
		m_shader->AddRef();
	}
	return *this;
}

Shader::~Shader()
{
	COM_FREE(m_shader);
}

const void* Shader::GetByteCode() const
{
	return &m_bytecode[0];
}

size_t Shader::GetByteCodeLen() const
{
	return m_bytecode.size();
}
