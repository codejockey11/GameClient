#pragma once

#include "framework.h"

class CDescriptorRange
{
public:

	D3D12_DESCRIPTOR_RANGE1* m_range;

	UINT m_count;
	UINT m_index;
	UINT m_numberOfDescriptors;

	CDescriptorRange();
	CDescriptorRange(UINT count);
	~CDescriptorRange();

	void SetRange(UINT index, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT count);
};