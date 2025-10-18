#pragma once

#include "framework.h"

class CRootParameter
{
public:

	D3D12_ROOT_PARAMETER1* m_rootParameter;
	
	UINT m_count;
	UINT m_index;

	CRootParameter();
	CRootParameter(UINT count);
	~CRootParameter();

	void Init32BitConstant(UINT index, UINT count);
	void InitDescriptorTable(UINT index, UINT count, D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges);
};

