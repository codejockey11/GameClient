#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CCommandAllocator
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	HRESULT m_hr;

	CCommandAllocator();
	CCommandAllocator(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CCommandAllocator();

	ComPtr<ID3D12GraphicsCommandList> CreateCommandList();
	ID3D12CommandAllocator* Get();
	void Reset();
};