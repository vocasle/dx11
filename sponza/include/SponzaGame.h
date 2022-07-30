#pragma once

#define _CRTDBG_MAP_ALLOC
#include <wrl/client.h>

#include <memory>
#include <vector>

#include "Camera.h"
#include "DeviceResources.h"
#include "Keyboard.h"
#include "LightHelper.h"
#include "ModelLoader.h"
#include "NE_Math.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "Timer.h"

class Game {
 public:
  Game();
  ~Game();

  void Tick();
  void Initialize(HWND hWnd, uint32_t width, uint32_t height);
  void GetDefaultSize(uint32_t *width, uint32_t *height);
  void OnWindowSizeChanged(int width, int height);

 private:
  void CreateDefaultSampler();
  void Clear();
  void Update();
  void Render();
  void CreateRasterizerState();
  void CreateWindowSizeDependentResources();

#if WITH_IMGUI
  void UpdateImgui();
#endif

  std::unique_ptr<DeviceResources> m_deviceResources;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;
  Timer m_timer;
  Camera m_camera;
  Renderer m_renderer;
  ShaderManager m_shaderManager;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
  ModelLoader m_modelLoader;
};
