#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CDepthBuffer
{
public:

	CErrorLog* m_errorLog;

	ComPtr<ID3D12DescriptorHeap> m_heap;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_resource;

	D3D12_CLEAR_VALUE m_clearValue;
	D3D12_CPU_DESCRIPTOR_HANDLE m_handle;
	D3D12_DEPTH_STENCIL_VIEW_DESC m_depthStencilViewDesc;
	D3D12_DEPTH_STENCIL_VIEW_DESC m_viewDesc;
	D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;

	HRESULT m_hr;

	int32_t m_height;
	int32_t m_width;

	CDepthBuffer();
	CDepthBuffer(CErrorLog* errorLog, ComPtr<ID3D12Device> device, int32_t numDescriptors, int32_t width, int32_t height);
	~CDepthBuffer();

	void Create();
	void CreateShadowMap();
	void SetPostDrawBarrier();
	void SetPreDrawBarrier();
};