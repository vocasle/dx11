#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include "Utils.h"

inline std::string foo(void* data, size_t sz)
{
	using namespace Microsoft::WRL;

	ComPtr<ID3D11ShaderReflection> reflector;
	D3DReflect(data, sz, IID_PPV_ARGS(&reflector));
	D3D11_SHADER_DESC shaderDesc = {};
	reflector->GetDesc(&shaderDesc);

	//shaderDesc.ConstantBuffers

	ID3D11ShaderReflectionConstantBuffer* cb = reflector->GetConstantBufferByName("PerObjectConstants");
	ID3D11ShaderReflectionVariable* mat = cb->GetVariableByName("world");

	D3D11_SHADER_VARIABLE_DESC desc = {};
	HR(mat->GetDesc(&desc))
	//free(mat);
	//free(cb);
	return UtilsFormatStr("Metadata :%s:%u\n", desc.Name, desc.Size);
}
