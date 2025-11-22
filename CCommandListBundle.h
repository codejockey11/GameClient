#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CCommandListBundle
{
public:

	enum
	{
		E_MAX_BUNDLES = 10,
		E_MAX_COMMAND_LIST_COUNT = 50
	};

	CErrorLog* m_errorLog;

	ComPtr<ID3D12GraphicsCommandList> m_commandLists[CCommandListBundle::E_MAX_COMMAND_LIST_COUNT];

	ID3D12CommandList* m_commandListPointers[CCommandListBundle::E_MAX_COMMAND_LIST_COUNT];

	int32_t m_count;

	CCommandListBundle();
	CCommandListBundle(CErrorLog* errorLog);
	~CCommandListBundle();

	void Add(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Clear();
	void Close();
};