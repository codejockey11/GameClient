#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CDepthBuffer
{
public:

	ComPtr<ID3D12DescriptorHeap> m_heap;
	ComPtr<ID3D12Resource> m_resource;
	
	D3D12_CLEAR_VALUE m_clearValue;
	D3D12_CPU_DESCRIPTOR_HANDLE m_handle;
	D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;
	D3D12_DEPTH_STENCIL_VIEW_DESC m_depthStencilViewDesc;

	int m_width;
	int m_height;

	CDepthBuffer();
	CDepthBuffer(CErrorLog* errorLog, ComPtr<ID3D12Device> device, UINT width, UINT height);
	~CDepthBuffer();

	void Create();
	void CreateShadowMap();
	void SetPostDrawBarrier();
	void SetPreDrawBarrier();

private:

	CErrorLog* m_errorLog;

	ComPtr<ID3D12Device> m_device;

	D3D12_HEAP_PROPERTIES m_heapProperties;

	D3D12_DEPTH_STENCIL_VIEW_DESC m_viewDesc;
};