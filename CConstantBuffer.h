#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeap.h"

#include "CGraphicsAdapter.h"

class CConstantBuffer
{
public:

	bool m_needsUpload;

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeap* m_values;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_buffer;
	ComPtr<ID3D12Resource> m_uploadHeap;

	D3D12_HEAP_PROPERTIES m_bufferHeapProperties;
	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_RESOURCE_ALLOCATION_INFO m_allocInfo;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_bufferDesc;
	D3D12_RESOURCE_DESC m_heapDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
	D3D12_SUBRESOURCE_DATA m_uploadData;

	HRESULT m_hr;

	int32_t m_size;

	uint64_t m_uploadBufferSize;

	void* m_heapAddress;

	CConstantBuffer();
	~CConstantBuffer();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, ComPtr<ID3D12GraphicsCommandList> commandList, int32_t size);
	void Reset();
	void UpdateBuffer();
};