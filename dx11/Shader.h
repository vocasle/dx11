#pragma once

#include <d3d11.h>
#include <vector>

enum class ShaderType
{
	Vertex,
	Pixel,
	Geometry,
	None
};

class Shader
{
public:
	Shader();
	Shader(const char* filepath, ID3D11Device* device, ShaderType type);
	Shader(const Shader& shader);
	Shader& operator=(const Shader& shader);
	~Shader();

	template <typename T>
	T GetAs() const;

	const void* GetByteCode() const;
	size_t GetByteCodeLen() const;

private:
	IUnknown* m_shader;
	ShaderType m_type;
	std::vector<uint8_t> m_bytecode;
};

template<typename T>
inline T Shader::GetAs() const
{
	return reinterpret_cast<T>(m_shader);
}
