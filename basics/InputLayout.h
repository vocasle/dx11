#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class InputLayout
{
public:
  enum class VertexType
  {
    Default = 56,
    Sky = 12,
    Particle = 24
  };

public:
  InputLayout ();
  void CreateDefaultLayout (ID3D11Device *device, unsigned char *bytes,
                            size_t bufferSize);
  void CreateSkyLayout (ID3D11Device *device, unsigned char *bytes,
                        size_t bufferSize);
  void CreateParticleLayout (ID3D11Device *device, unsigned char *bytes,
                             size_t bufferSize);

  ID3D11InputLayout *
  GetDefaultLayout () const
  {
    return m_defaultLayout.Get ();
  }
  ID3D11InputLayout *
  GetSkyLayout () const
  {
    return m_skyLayout.Get ();
  }
  ID3D11InputLayout *
  GetParticleLayout () const
  {
    return m_particleLayout.Get ();
  }
  static size_t GetVertexSize (VertexType vertexType);

private:
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_defaultLayout;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyLayout;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_particleLayout;
};
