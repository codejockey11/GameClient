#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CIndexBuffer
{
public:

	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	D3D12_INDEX_BUFFER_VIEW	m_view;

	UINT m_count;

	CIndexBuffer();
	CIndexBuffer(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, UINT count, void* indicies);
	~CIndexBuffer();

	BYTE CreateResource(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	void Draw();
	void LoadBuffer(void* indices);

	void SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) { m_commandList = commandList; }

private:

	D3D12_HEAP_PROPERTIES m_heapProperties;

	D3D12_RESOURCE_DESC m_resourceDesc;
};