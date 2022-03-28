#pragma once

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <stdint.h>

#define DEFAULT_WIN_WIDTH 1024
#define DEFAULT_WIN_HEIGHT 720

typedef struct DeviceResources
{
	ID3D11Device1* Device;
	ID3D11DeviceContext1* Context;
	IDXGISwapChain1* SwapChain;
	IDXGIFactory2* Factory;
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11DepthStencilView* DepthStencilView;
	ID3D11Texture2D* RenderTarget;
	ID3D11Texture2D* DepthStencil;
	ID3D11RasterizerState1* RasterizerState;
	uint32_t BackbufferWidth;
	uint32_t BackbufferHeight;
	uint32_t MultiSampleQualityLevel;
	D3D11_VIEWPORT ScreenViewport;
	HWND hWnd;
	RECT OutputSize;
} DeviceResources;

DeviceResources* DRNew(void);

void DRFree(DeviceResources* device);

void DRSetWindow(DeviceResources* dr, HWND hWnd, int width, int height);

void DRCreateDeviceResources(DeviceResources* dr);

void DRCreateWindowSizeDependentResources(DeviceResources* dr);

void DRReportLiveObjects(void);