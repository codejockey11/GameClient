#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CCommandAllocator.h"
#include "CGraphicsAdapter.h"

class CBundle
{
public:

	enum
	{
		E_MAX_COMMAND_LIST = 50
	};

	CCommandAllocator* m_commandAllocator;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12GraphicsCommandList> m_commandLists[CBundle::E_MAX_COMMAND_LIST];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	ID3D12CommandList* m_commandListPointers[CBundle::E_MAX_COMMAND_LIST];

	int32_t m_count;

	CBundle();
	CBundle(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CBundle();

	void CloseAndExecute();
	ComPtr<ID3D12CommandList> CreateCommandList();
	void Reset();
};