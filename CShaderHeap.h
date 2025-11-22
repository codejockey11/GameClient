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
	D3D12_DESCRIPTOR_HEAP_DESC m_desc;

	HRESULT m_hr;

	CShaderHeap();
	CShaderHeap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t descriptorCount);
	~CShaderHeap();
};