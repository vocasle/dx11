#pragma once

#define _CRTDBG_MAP_ALLOC
#include "DeviceResources.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "NE_Math.h"
#include "Renderer.h"
#include "Timer.h"

#include <memory>
#include <vector>
#include <wrl/client.h>

#define MODEL_PULL 10
#define TEXTURE_PULL 4

class Game
{
public:
  Game ();
  ~Game ();

  void Tick ();
  void Initialize (HWND hWnd, uint32_t width, uint32_t height);
  void GetDefaultSize (uint32_t *width, uint32_t *height);

private:
  void CreateDefaultSampler ();
  void Clear ();
  void Update ();
  void Render ();
  std::vector<uint8_t> CreateVertexShader (const char *filepath,
                                           ID3D11Device *device,
                                           ID3D11VertexShader **vs);
  void CreatePixelShader (const char *filepath, ID3D11Device *device,
                          ID3D11PixelShader **ps);

#if WITH_IMGUI
  void UpdateImgui ();
#endif

  std::unique_ptr<DeviceResources> m_DR;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PS;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_IL;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_DefaultSampler;
  Timer m_Timer;
  Renderer m_Renderer;
};
