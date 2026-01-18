#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeap.h"

#include "CCommandAllocator.h"
#include "CGraphicsAdapter.h"

class CConstantBuffer
{
public:

	bool m_needsUpload;

	CCommandAllocator* m_commandAllocator;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeap* m_values;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_gpuBuffer;
	ComPtr<ID3D12Resource> m_cpuBuffer;

	D3D12_HEAP_PROPERTIES m_heapProp;
	D3D12_RESOURCE_ALLOCATION_INFO m_allocInfo;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
	D3D12_SUBRESOURCE_DATA m_uploadData;

	HRESULT m_hr;

	int32_t m_size;

	uint64_t m_uploadBufferSize;

	void* m_heapAddress;

	CConstantBuffer();
	~CConstantBuffer();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t size);
	void CreateDynamicResource();
	void CreateStaticResource();
	void LoadDynamicBuffer();
	void RecordStatic();
	void ReleaseStaticCPUResource();
	void Reset();
	void UploadStaticResources();
};