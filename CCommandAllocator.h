#pragma once

#include "framework.h"

#include "CGraphicsAdapter.h"

class CCommandAllocator
{
public:

	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator[CGraphicsAdapter::E_BACKBUFFER_COUNT];

	CCommandAllocator();
	CCommandAllocator(CGraphicsAdapter* graphicsAdapter);
	~CCommandAllocator();

	ID3D12CommandAllocator* Get();
	void Reset();
};