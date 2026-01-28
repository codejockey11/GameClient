#pragma once

#include "framework.h"

#include "../GameCommon/CRGBColor.h"
#include "../GameCommon/CErrorLog.h"

#ifdef D3D11on12
#include "CD3D11On12.h"
#endif

#include "CDepthBuffer.h"
#include "CDXGINames.h"
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

	CDepthBuffer* m_swapChainDepthBuffers[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CDXGINames* m_DXGINames;
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
	IDXGIDebug* m_debug;
#endif

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Fence> m_fence;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12InfoQueue> m_infoQueue;
	ComPtr<IDXGIAdapter4> m_adapter;
	ComPtr<IDXGIFactory6> m_factory;
	ComPtr<IDXGIOutput> m_adapterOutput;
	ComPtr<IDXGISwapChain3> m_swapChain;

	CRenderTarget* m_swapChainRenderTargets[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CRGBColor* m_rgbColor;
	CString* m_adapterDescription;
	CViewport* m_swapChainViewport;

	D3D_FEATURE_LEVEL m_featureLevel;
	D3D_FEATURE_LEVEL m_featureCheck;

	D3D12_COMMAND_QUEUE_DESC m_commandQueueDesc;
	D3D12_FEATURE_DATA_ARCHITECTURE1 m_featureDataArchitecture;
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

	POINT m_middle;

	RECT m_desktop;
	RECT m_desktopRect;
	RECT m_fullScreenWindowRect;
	RECT m_windowRect;

	int32_t m_backbufferIndex;
	int32_t m_descriptorHandleIncrementSize;
	int32_t m_factoryFlags;
	int32_t m_height;
	int32_t m_posX;
	int32_t m_posY;
	int32_t m_width;

	uint32_t m_numModes;

	uint64_t m_currentFenceValue;
	uint64_t m_fenceValues[CGraphicsAdapter::E_BACKBUFFER_COUNT];

	XMFLOAT4X4 m_wvpMat;

	CGraphicsAdapter();
	CGraphicsAdapter(CErrorLog* errorLog, HWND hWnd, int32_t width, int32_t height, bool windowed, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat);
	~CGraphicsAdapter();

	void Create11on12Device();
	void CreateCommandQueue();
	void CreateDevice();
	void CreateFence();
	void CreateRenderTargets();
	void CreateSwapChain();
	void DeviceInformation();
	void DXGIFactory();
	void End2DDraw();
	D3D12_CPU_DESCRIPTOR_HANDLE* GetDepthBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTarget();
	D3D12_RECT* GetScissorRect();
	D3D12_VIEWPORT* GetViewport();
	void Init2DDraw();
	void MoveToNextFrame();
	void Record();
	void ToggleFullScreenWindow();
	void WaitForGPU();
};