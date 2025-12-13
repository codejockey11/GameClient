#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CShaderHeap
{
public:

	enum
	{
		E_MAX_CONSTANT_BUFFERS = 14,
		E_MAX_TEXTURE_BUFFERS = 128
	};

	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12DescriptorHeap> m_heap;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptorHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptorHandleForHeapStart;
	D3D12_DESCRIPTOR_HEAP_DESC m_desc;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescriptorHandleForHeapStart;

	HRESULT m_hr;

	CShaderHeap();
	CShaderHeap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t descriptorCount);
	~CShaderHeap();

	void BindResource(int32_t location, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc);
	D3D12_GPU_DESCRIPTOR_HANDLE GetBaseDescriptor();
	ID3D12DescriptorHeap* const* GetDescriptorHeap();
};