#include <d3d11.h>
#include <wrl/client.h>

#include <unordered_map>
#include <string>
#include <vector>

#include "InputLayout.h"

class ShaderManager
{
public:
	void Initialize(ID3D11Device* device);
	size_t GetStrides() const;
	ID3D11InputLayout* GetInputLayout() const;
	ID3D11VertexShader* GetVertexShader(const std::string& name);
	ID3D11PixelShader* GetPixelShader(const std::string& name) const;

private:
	void CreateVertexShader(const std::string& filepath, ID3D11Device* device);
	void CreatePixelShader(const std::string& filepath, ID3D11Device* device);

private:
	std::string m_activeVS;

	typedef std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11VertexShader>> VertexShaderMap;
	typedef std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11PixelShader>> PixelShaderMap;
	VertexShaderMap m_vertexShaders;
	PixelShaderMap m_pixelShaders;
	typedef std::unordered_map<std::string, InputLayout> InputLayoutMap;
	InputLayoutMap m_inputLayouts;
};
