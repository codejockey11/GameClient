#pragma once

#include "framework.h"

class CDescriptorTable
{
public:

	enum
	{
		MAX_RANGE = 2
	};

	D3D12_ROOT_DESCRIPTOR_TABLE1 m_descriptorTable[CDescriptorTable::MAX_RANGE];

	UINT m_index;

	CDescriptorTable();
	~CDescriptorTable();

	void SetRange(UINT index, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT count);
};