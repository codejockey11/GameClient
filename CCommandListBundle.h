#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CCommandListBundle
{
public:

	enum
	{
		E_MAX_BUNDLES = 5,
		E_MAX_COMMAND_LIST_COUNT = 50
	};

	ID3D12CommandList* m_commandListPointers[CCommandListBundle::E_MAX_COMMAND_LIST_COUNT];

	UINT m_count;

	ComPtr<ID3D12GraphicsCommandList> m_commandLists[CCommandListBundle::E_MAX_COMMAND_LIST_COUNT];

	CCommandListBundle();
	CCommandListBundle(CErrorLog* errorLog);
	~CCommandListBundle();

	void Add(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Clear();
	void Close();

private:

	CErrorLog* m_errorLog;
};