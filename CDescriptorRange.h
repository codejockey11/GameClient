#pragma once

#include "framework.h"

class CDescriptorRange
{
public:

	D3D12_DESCRIPTOR_RANGE1* m_range;

	int32_t m_count;
	int32_t m_index;
	int32_t m_numberOfDescriptors;

	CDescriptorRange();
	CDescriptorRange(int32_t count);
	~CDescriptorRange();

	void SetRange(int32_t index, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, int32_t count);
};