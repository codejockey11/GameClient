#pragma once

#include "framework.h"

#include "../GameCommon/CColor.h"
#include "../GameCommon/CErrorLog.h"

#include "CCommandListBundle.h"
#ifdef D3D11on12
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

	bool m_vsyncEnabled;
	bool m_windowed;

	CColor* m_color;
	CCommandListBundle* m_commandListBundle[CCommandListBundle::E_MAX_BUNDLES];
	CDepthBuffer* m_swapChainDepthBuffers[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CErrorLog* m_errorLog;

#ifdef D3D11on12
	CD3D11On12* m_d3d11On12;
#else
	ComPtr<ID3D12GraphicsCommandList> m_eofCommandList;
#endif

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> m_debugController;
	IDXGIDebug1* m_debug;
#endif
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Fence> m_fence;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12InfoQueue> m_infoQueue;
	ComPtr<IDXGIAdapter1> m_adapter;
	ComPtr<IDXGIFactory6> m_factory;
	ComPtr<IDXGIOutput> m_adapterOutput;
	ComPtr<IDXGISwapChain3> m_swapChain;

	CRenderTarget* m_swapChainRenderTargets[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CString* m_adapterDescription;
	CViewport* m_swapChainViewport;

	D3D12_COMMAND_QUEUE_DESC m_commandQueueDesc;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS  m_sampleQualityLevels;
	D3D12_INFO_QUEUE_FILTER m_newFilter;
	D3D12_MESSAGE_ID* m_denyIds;
	D3D12_MESSAGE_SEVERITY* m_severities;
	DXGI_ADAPTER_DESC m_adapterDesc;
	DXGI_FORMAT m_backBufferFormat;
	DXGI_FORMAT m_depthStencilFormat;
	DXGI_MODE_DESC* m_displayModeList;
	DXGI_SWAP_CHAIN_DESC m_swapChainDesc;

	float m_dpi;
	float m_heightRatio;
	float m_widthRatio;

	HANDLE m_fenceEvent;

	HMENU m_menu;

	HRESULT m_hr;

	HWND m_hWnd;

	int m_height;
	int m_width;

	POINT m_middle;

	RECT m_desktop;
	RECT m_desktopRect;
	RECT m_fullScreenWindowRect;
	RECT m_windowRect;

	UINT m_backbufferIndex;
	UINT m_descriptorHandleIncrementSize;
	UINT m_factoryFlags;
	UINT m_numModes;
	UINT m_posX;
	UINT m_posY;
	UINT m_tearingSupport;

	UINT64 m_currentFenceValue;
	UINT64 m_fenceValues[CGraphicsAdapter::E_BACKBUFFER_COUNT];

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
	D3D12_CPU_DESCRIPTOR_HANDLE* GetDepthBuffer();
	void GetHardwareAdapter();
	char* GetDXGIFormatName(DXGI_FORMAT f);
	D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTarget();
	D3D12_RECT* GetScissorRect();
	D3D12_VIEWPORT* GetViewport();
	void Init2DDraw();
	void MoveToNextFrame();
	void ResetAndClearTargets();
	void ToggleFullScreenWindow();
	void WaitForGPU();
};