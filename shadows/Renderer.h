#pragma once

#include <cstdint>
#include <d3d11.h>

#include "DeviceResources.h"

#define R_MAX_SRV_NUM 5
#define R_MAX_CB_NUM 3
#define R_MAX_SAMPLERS 2

#define R_DEFAULT_PRIMTIVE_TOPOLOGY D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST

enum class BindTargets
{
  PixelShader,
  VertexShader
};

class Renderer
{
public:
  Renderer ();
  ~Renderer ();

  void SetDeviceResources (DeviceResources *dr);
  void SetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY topology);
  void SetInputLayout (ID3D11InputLayout *inputLayout);
  void SetRasterizerState (ID3D11RasterizerState *rasterizerState);
  void SetSamplerState (ID3D11SamplerState *state, uint32_t slot);

  void BindPixelShader (ID3D11PixelShader *shader);
  void BindVertexShader (ID3D11VertexShader *shader);
  void BindShaderResources (enum BindTargets bindTarget,
                            ID3D11ShaderResourceView **SRVs, uint32_t numSRVs);
  void BindConstantBuffers (enum BindTargets bindTarget, ID3D11Buffer **CBs,
                            uint32_t numCBs);
  void BindShaderResource (enum BindTargets bindTarget,
                           ID3D11ShaderResourceView *srv, uint32_t slot);
  void BindConstantBuffer (enum BindTargets bindTarget, ID3D11Buffer *cb,
                           uint32_t slot);

  void DrawIndexed (ID3D11Buffer *indexBuffer, ID3D11Buffer *vertexBuffer,
                    uint32_t strides, uint32_t indexCount,
                    uint32_t startIndexLocation, uint32_t baseVertexLocation);
  void Clear ();
  void Present ();

private:
  D3D11_PRIMITIVE_TOPOLOGY m_Topology;
  ID3D11InputLayout *m_InputLayout;
  ID3D11RasterizerState *m_RasterizerState;
  ID3D11SamplerState *m_SamplerStates[R_MAX_SAMPLERS];
  ID3D11PixelShader *m_PS;
  ID3D11VertexShader *m_VS;
  ID3D11ShaderResourceView *m_PS_SRV[R_MAX_SRV_NUM];
  ID3D11Buffer *m_PS_CB[R_MAX_CB_NUM];
  ID3D11Buffer *m_VS_CB[R_MAX_CB_NUM];
  DeviceResources *m_DR;
};
