#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CCommandListGroup
{
public:

	enum
	{
		E_MAX_LIST_GROUP = 10,
		E_MAX_LISTS = 100
	};

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12GraphicsCommandList> m_commandLists[CCommandListGroup::E_MAX_LISTS];

	ID3D12CommandList* m_commandListPointers[CCommandListGroup::E_MAX_LISTS];

	int32_t m_count;

	CCommandListGroup();
	CCommandListGroup(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CCommandListGroup();

	void Add(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Clear();
	void Close();
	void ExecuteCommandLists();
};