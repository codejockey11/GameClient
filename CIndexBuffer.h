#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CIndexBuffer
{
public:

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_indexBuffer;

	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_INDEX_BUFFER_VIEW	m_view;
	D3D12_RESOURCE_DESC m_resourceDesc;

	HRESULT m_hr;

	UINT m_count;

	void* m_heapAddress;

	CIndexBuffer();
	CIndexBuffer(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, UINT count, void* indicies);
	~CIndexBuffer();

	void Record();
	void SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
};