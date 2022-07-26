#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class InputLayout {
    public:
	enum class VertexType { Default = 56, Sky = 12 };

    public:
	InputLayout();
	void CreateDefaultLayout(ID3D11Device *device, unsigned char *bytes,
				 size_t bufferSize);
	void CreateSkyLayout(ID3D11Device *device, unsigned char *bytes,
			     size_t bufferSize);

	ID3D11InputLayout *GetDefaultLayout() const
	{
		return m_defaultLayout.Get();
	}
	ID3D11InputLayout *GetSkyLayout() const
	{
		return m_skyLayout.Get();
	}
	size_t GetVertexSize(VertexType vertexType) const;

    private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_defaultLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyLayout;
};
