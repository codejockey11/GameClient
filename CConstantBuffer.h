#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeap.h"

#include "CGraphicsAdapter.h"

class CConstantBuffer
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeap* m_values;

	bool m_needsUpload;

	HRESULT m_hr;

	int m_size;

	ComPtr<ID3D12Resource> m_buffer;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;

	CConstantBuffer();
	~CConstantBuffer();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, UINT size, ComPtr<ID3D12GraphicsCommandList> commandList);
	void Reset();
	void SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) { m_commandList = commandList; }
	void UpdateConstantBuffer();

private:

	ComPtr<ID3D12Resource> m_uploadHeap;

	D3D12_HEAP_PROPERTIES m_bufferHeapProperties;
	D3D12_HEAP_PROPERTIES m_heapProperties;

	D3D12_RESOURCE_ALLOCATION_INFO m_allocInfo;

	D3D12_RESOURCE_DESC m_bufferDesc;
	D3D12_RESOURCE_DESC m_heapDesc;

	D3D12_SUBRESOURCE_DATA m_uploadData;

	UINT64 m_uploadBufferSize;
};