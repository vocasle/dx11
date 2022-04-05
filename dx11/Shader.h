#pragma once

#include <d3d11.h>

enum class ShaderType
{
	Vertex,
	Pixel,
	Geometry
};

class Shader
{
public:
	Shader(const char* filepath, ID3D11Device* device, ShaderType type);
	~Shader();

	template <typename T>
	T* GetAs() const;

private:
	IUnknown* m_shader;
	ShaderType m_type;
};

template<typename T>
inline T* Shader::GetAs() const
{
	return reinterpret_cast<T*>(m_shader);
}
