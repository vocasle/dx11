#include "DeviceResources.h"
#include "Utils.h"

#include <combaseapi.h>


DeviceResources* DRNew(void)
{
	DeviceResources* dr = (DeviceResources*) malloc(sizeof(DeviceResources));
	memset(dr, 0, sizeof(DeviceResources));
	dr->BackbufferHeight = DEFAULT_WIN_HEIGHT;
	dr->BackbufferWidth = DEFAULT_WIN_WIDTH;
	
	return dr;
}

void DRFree(DeviceResources* dr)
{
	COM_FREE(dr->Device);
	COM_FREE(dr->Context);
	COM_FREE(dr->SwapChain);
	COM_FREE(dr->Factory);
	COM_FREE(dr->RenderTargetView);
	COM_FREE(dr->RenderTarget);
	COM_FREE(dr->DepthStencilView);
	COM_FREE(dr->DepthStencil);
	COM_FREE(dr->RasterizerState);
	memset(dr, 0, sizeof(DeviceResources));

	free(dr);
}

void DRSetWindow(DeviceResources* dr, HWND hWnd, int width, int height)
{
	dr->hWnd = hWnd;
	dr->OutputSize.right = width;
	dr->OutputSize.bottom = height;
}

void DRCreateFactory(DeviceResources* dr)
{
	UINT dxgiFactoryFlags = 0;
#if _DEBUG
	{
		IDXGIInfoQueue* infoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, &IID_IDXGIInfoQueue, (void**)&infoQueue)))
		{
			dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			infoQueue->SetBreakOnSeverity(infoQueue, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(infoQueue, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			COM_FREE(infoQueue);
		}
	}
#endif
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, &IID_IDXGIFactory, (void**)&dr->Factory)))
	{
		OutputDebugStringA("ERROR: Failed to create DXGI factory\n");
		ExitProcess(EXIT_FAILURE);
	}
}

void DRCreateRasterizerState(DeviceResources* dr)
{
	D3D11_RASTERIZER_DESC1 rasterizerDesc = {0};
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	if (FAILED(dr->Device->CreateRasterizerState1(dr->Device, &rasterizerDesc, &dr->RasterizerState)))
	{
		OutputDebugStringA("ERROR: Failed to create rasterizer state\n");
		ExitProcess(EXIT_FAILURE);
	}
}

void DRCreateDeviceResources(DeviceResources* dr)
{
	UINT createDeviceFlag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if _DEBUG
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DRCreateFactory(dr);

	const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	IDXGIAdapter1* adapter;

	if (FAILED(dr->Factory->EnumAdapters1(dr->Factory, 0, &adapter)))
	{
		OutputDebugStringA("ERROR: Failed to enumerate adapter\n");
		ExitProcess(EXIT_FAILURE);
	}

	if (FAILED(D3D11CreateDevice((IDXGIAdapter*)adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
		createDeviceFlag,
		featureLevels,
		sizeof(featureLevels) / sizeof(featureLevels[0]),
		D3D11_SDK_VERSION,
		(ID3D11Device**)&dr->Device,
		NULL,
		(ID3D11DeviceContext**)&dr->Context)))
	{
		OutputDebugStringA("ERROR: Failed to create device\n");
		ExitProcess(EXIT_FAILURE);
	}

	COM_FREE(adapter);
}

void DRCreateWindowSizeDependentResources(DeviceResources* dr)
{
	if (!dr->hWnd)
	{
		OutputDebugStringA("ERROR: hWnd is not set!\n");
		ExitProcess(EXIT_FAILURE);
	}

	ID3D11DeviceContext1* ctx = dr->Context;

	ID3D11RenderTargetView* nullViews[] = { NULL };
	ctx->OMSetRenderTargets(ctx, _countof(nullViews), nullViews, NULL);
	if (dr->RenderTargetView)
		COM_FREE(dr->RenderTargetView);
	if (dr->DepthStencilView)
		COM_FREE(dr->DepthStencilView);
	if (dr->RenderTarget)
		COM_FREE(dr->RenderTarget);
	if (dr->DepthStencil)
		COM_FREE(dr->DepthStencil);
	ctx->Flush(ctx);

	const UINT backBufferWidth = max(dr->OutputSize.right - dr->OutputSize.left, 1u);
	const UINT backBufferHeight = max(dr->OutputSize.bottom - dr->OutputSize.top, 1u);
	const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

	UINT numQualityLevels = 0;
	if (FAILED(dr->Device->CheckMultisampleQualityLevels(dr->Device, DXGI_FORMAT_B8G8R8A8_UNORM, 4, &numQualityLevels)))
	{
		OutputDebugStringA("ERROR: Failed to query multisample quality levels\n");
		ExitProcess(EXIT_FAILURE);
	}
	dr->MultiSampleQualityLevel = numQualityLevels;

	if (dr->SwapChain)
	{
		if (FAILED(dr->SwapChain->ResizeBuffers(dr->SwapChain, 2, backBufferWidth, backBufferHeight, backBufferFormat, 0)))
		{
			OutputDebugStringA("ERROR: Failed to resize buffers\n");
			ExitProcess(EXIT_FAILURE);
		}
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SampleDesc.Count = dr->MultiSampleQualityLevel;
		swapChainDesc.SampleDesc.Quality = dr->MultiSampleQualityLevel - 1;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {0};
		fsSwapChainDesc.Windowed = TRUE;

		// Create a SwapChain from a Win32 window.
		if (FAILED(dr->Factory->CreateSwapChainForHwnd(dr->Factory,
			(IUnknown*)dr->Device,
			dr->hWnd,
			&swapChainDesc,
			&fsSwapChainDesc,
			NULL,
			&dr->SwapChain
		)))
		{
			OutputDebugStringA("ERROR: Failed to create swap chain\n");
			ExitProcess(EXIT_FAILURE);
		}

		// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
		if (FAILED(dr->Factory->MakeWindowAssociation(dr->Factory, dr->hWnd, DXGI_MWA_NO_ALT_ENTER)))
		{
			OutputDebugStringA("ERROR: Failed to make window association\n");
			ExitProcess(EXIT_FAILURE);
		}
	}

	if (FAILED(dr->SwapChain->GetBuffer(dr->SwapChain, 0, &IID_ID3D11Texture2D, (void**)&dr->RenderTarget)))
	{
		OutputDebugStringA("ERROR: Failed to get render target\n");
		ExitProcess(EXIT_FAILURE);
	}

	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { 0 };
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		if (FAILED(dr->Device->CreateRenderTargetView(dr->Device, (ID3D11Resource*)dr->RenderTarget, &renderTargetViewDesc, &dr->RenderTargetView)))
		{
			OutputDebugStringA("ERROR: Failed to create render target view\n");
			ExitProcess(EXIT_FAILURE);
		}
	}

	{
		D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
		depthStencilDesc.Width = backBufferWidth;
		depthStencilDesc.Height = backBufferHeight;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;

		ID3D11Device1* device = dr->Device;

		if (FAILED(device->CreateTexture2D(device, &depthStencilDesc, NULL, &dr->DepthStencil)))
		{
			OutputDebugStringA("ERROR: Failed to create depth stencil texture\n");
			ExitProcess(EXIT_FAILURE);
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { 0 };
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Format = depthStencilDesc.Format;

		if (FAILED(device->CreateDepthStencilView(device, (ID3D11Resource*)dr->DepthStencil, &depthStencilViewDesc, &dr->DepthStencilView)))
		{
			OutputDebugStringA("ERROR: Failed to create depth stencil view\n");
			ExitProcess(EXIT_FAILURE);
		}
	}

	dr->ScreenViewport.TopLeftX = dr->ScreenViewport.TopLeftY = 0.0f;
	dr->ScreenViewport.Width = (float)backBufferWidth;
	dr->ScreenViewport.Height = (float)backBufferHeight;
	dr->ScreenViewport.MinDepth = 0;
	dr->ScreenViewport.MaxDepth = 1;

	DRCreateRasterizerState(dr);
}

void DRReportLiveObjects(void)
{
#ifdef _DEBUG
	{
		IDXGIDebug1* dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, &IID_IDXGIDebug1,(void**)&dxgiDebug)))
		{
			dxgiDebug->ReportLiveObjects(dxgiDebug, DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL);
		}
	}
#endif
}
