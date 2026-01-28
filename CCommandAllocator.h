#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CCommandAllocator
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12CommandAllocator>* m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	HRESULT m_hr;

	int32_t m_bufferCount;

	CCommandAllocator();
	CCommandAllocator(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t bufferCount);
	~CCommandAllocator();

	ComPtr<ID3D12GraphicsCommandList> CreateCommandList();
	ID3D12CommandAllocator* Get();
	void Reset();
};