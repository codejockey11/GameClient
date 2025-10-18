#pragma once

#include "framework.h"

#include "../GameCommon/CColor.h"
#include "../GameCommon/CErrorLog.h"

#include "CCommandListBundle.h"
#ifdef DX11on12
#include "CD3D11On12.h"
#endif
#include "CDepthBuffer.h"
#include "CRenderTarget.h"
#include "CViewport.h"

class CGraphicsAdapter
{
public:

	enum
	{
		E_BACKBUFFER_COUNT = 2
	};

	CColor* m_color;
	CCommandListBundle* m_commandListBundle[CCommandListBundle::E_MAX_BUNDLES];
#ifdef DX11on12
	CD3D11On12* m_d3d11On12;
#endif
	CDepthBuffer* m_swapChainDepthBuffers[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CRenderTarget* m_swapChainRenderTargets[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CString* m_adapterDescription;
	CViewport* m_swapChainViewport;

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
#ifndef DX11on12
	ComPtr<ID3D12GraphicsCommandList> m_eofCommandList;
#endif
	ComPtr<IDXGISwapChain3> m_swapChain;
	
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS  m_sampleQualityLevels;
	DXGI_FORMAT m_backBufferFormat;
	DXGI_FORMAT m_depthStencilFormat;

	bool m_windowed;

	HMENU m_menu;
	HWND m_hWnd;

	int m_height;
	int m_width;
	
	float m_dpi;
	float m_widthRatio;
	float m_heightRatio;

	POINT m_middle;

	RECT m_desktop;

	UINT m_backbufferIndex;
	UINT m_posX;
	UINT m_posY;
	UINT m_tearingSupport;
	UINT m_descriptorHandleIncrementSize;

	CGraphicsAdapter();
	CGraphicsAdapter(CErrorLog* errorLog, HWND hWnd, int width, int height, bool windowed, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat);
	~CGraphicsAdapter();

	void BundleCommandList(int bundleNumber, ComPtr<ID3D12GraphicsCommandList> commandList);
	void Create11on12Device();
	void CreateCommandQueue();
	void CreateDevice();
	void CreateFence();
	void CreateRenderTargets();
	void CreateSwapChain();
	void DeviceInformation();
	void DXGIFactory();
	void End2DDraw();
	void ExecuteCommandLists();
	char* GetDXGIFormatName(DXGI_FORMAT f);
	void GetHardwareAdapter();
	void Init2DDraw();
	void MoveToNextFrame();
	void ResetAndClearTargets();
	void ToggleFullScreenWindow();
	void WaitForGPU();

private:

	CErrorLog* m_errorLog;

	HRESULT m_hr;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> m_debugController;
#endif
	ComPtr<ID3D12Fence> m_fence;
	ComPtr<ID3D12InfoQueue> m_infoQueue;
	ComPtr<IDXGIAdapter1> m_adapter;
	ComPtr<IDXGIFactory6> m_factory;
	ComPtr<IDXGIOutput> m_adapterOutput;

	bool m_vsyncEnabled;

	D3D12_COMMAND_QUEUE_DESC m_commandQueueDesc;

	DXGI_ADAPTER_DESC m_adapterDesc;
	DXGI_MODE_DESC* m_displayModeList;
	DXGI_SWAP_CHAIN_DESC m_swapChainDesc;

	HANDLE m_fenceEvent;

	RECT m_desktopRect;
	RECT m_fullScreenWindowRect;
	RECT m_windowRect;

	UINT m_factoryFlags;
	UINT m_numModes;

	UINT64 m_currentFenceValue;
	UINT64 m_fenceValues[CGraphicsAdapter::E_BACKBUFFER_COUNT];
};