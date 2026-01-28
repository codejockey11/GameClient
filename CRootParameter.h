#pragma once

#include "framework.h"

class CRootParameter
{
public:

	D3D12_ROOT_PARAMETER1* m_rootParameter;
	
	int32_t m_count;

	CRootParameter();
	CRootParameter(int32_t count);
	~CRootParameter();

	void Init32BitConstant(int32_t index, int32_t count);
	void InitDescriptorTable(int32_t index, int32_t count, D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges);
};