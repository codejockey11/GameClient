#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CRenderTarget
{
public:
	
	CErrorLog* m_errorLog;

	ComPtr<ID2D1Bitmap1> m_d2dRenderTarget;
	ComPtr<ID2D1DeviceContext1> m_d2dDeviceContext;
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<ID3D11Resource> m_wrappedBackBuffer;
	ComPtr<ID3D12DescriptorHeap> m_heap;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_resource;
	ComPtr<IDXGISurface> m_surface;

	D2D1_BITMAP_PROPERTIES1 m_bitmapProperties;
	D2D1_SIZE_F m_rtSize;

#ifdef D3D11on12
	D3D11_RESOURCE_FLAGS m_d3d11Flags;
#endif

	D3D12_CLEAR_VALUE m_clearValue;
	D3D12_CPU_DESCRIPTOR_HANDLE	m_handle;
	D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;

	HRESULT m_hr;

	CRenderTarget();
	CRenderTarget(CErrorLog* errorLog, ComPtr<ID3D12Device> device);
	~CRenderTarget();

	void AdditionalTarget(int32_t width, int32_t height, DXGI_FORMAT format);
	void SetPostDrawBarrier();
	void SetPreDrawBarrier();
	void SwapChainTarget(ComPtr<IDXGISwapChain3> swapChain, int32_t bufferNbr);

#ifdef D3D11on12
	void WrapResource(float dpi, ComPtr<ID3D11On12Device> d3d11On12Device, ComPtr<ID2D1DeviceContext1> d2dDeviceContext);
#endif
};