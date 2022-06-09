#include "ShaderManager.h"

#include "SoftShadowsConfig.h"
#include "Utils.h"

#include <Windows.h>

using namespace Microsoft::WRL;

void ShaderManager::Initialize(ID3D11Device* device)
{
	//SHADERS_ROOT
	WIN32_FIND_DATA findData = {};
	const std::string shadersRoot = UtilsFormatStr("%s\\*", SHADERS_ROOT);
	HANDLE f = FindFirstFile(shadersRoot.c_str(), &findData);
	do
	{
		const std::string shaderName = findData.cFileName;
		if (shaderName.find(".cso") != std::string::npos)
		{
			UtilsDebugPrint("FOUND SHADER: %s\n", shaderName.c_str());
			if (shaderName.find("VS") != std::string::npos)
			{
				CreateVertexShader(shaderName, device);
			}
			else if (shaderName.find("PS") != std::string::npos)
			{
				CreatePixelShader(shaderName, device);
			}
		}
	}
	while (FindNextFile(f, &findData));
	FindClose(f);
}

size_t ShaderManager::GetStrides() const
{
	return m_activeVS.empty() ? 0 : m_inputLayouts.at(m_activeVS).GetStrides();
}

ID3D11InputLayout* ShaderManager::GetInputLayout() const
{
	return m_activeVS.empty() ? nullptr : m_inputLayouts.at(m_activeVS).Get();
}

ID3D11VertexShader* ShaderManager::GetVertexShader(const std::string& name)
{
	const auto it = m_vertexShaders.find(name);
	if (it != std::end(m_vertexShaders))
	{
		m_activeVS = it->first;
		return it->second.Get();
	}
	return nullptr;
}

ID3D11PixelShader* ShaderManager::GetPixelShader(const std::string& name) const
{
	const auto it = m_pixelShaders.find(name);
	return it == std::end(m_pixelShaders) ? nullptr : it->second.Get();
}

void ShaderManager::CreateVertexShader(const std::string& filepath, ID3D11Device* device)
{
	const auto bytes = UtilsReadData(UtilsFormatStr("%s/%s", SHADERS_ROOT, filepath.c_str()).c_str());
	ComPtr<ID3D11VertexShader> vs;

	if (FAILED(device->CreateVertexShader(&bytes[0], bytes.size(), NULL, vs.GetAddressOf())))
	{
		UtilsDebugPrint("ERROR: Failed to create vertex shader from %s", filepath.c_str());
		return;
	}

	const std::string shaderName = filepath.substr(0, filepath.find_last_not_of("cso"));
	m_vertexShaders[shaderName] = vs;
	m_inputLayouts[shaderName] = InputLayout(device, &bytes[0], bytes.size());
}

void ShaderManager::CreatePixelShader(const std::string& filepath, ID3D11Device* device)
{
	const auto bytes = UtilsReadData(UtilsFormatStr("%s/%s", SHADERS_ROOT, filepath.c_str()).c_str());
	ComPtr<ID3D11PixelShader> ps;

	if (FAILED(device->CreatePixelShader(&bytes[0], bytes.size(), NULL, ps.GetAddressOf())))
	{
		UtilsDebugPrint("ERROR: Failed to create pixel shader from %s", filepath.c_str());
		return;
	}

	const std::string shaderName = filepath.substr(0, filepath.find_last_not_of("cso"));
	m_pixelShaders[shaderName] = ps;
}
