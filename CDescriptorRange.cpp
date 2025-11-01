#include "CDescriptorRange.h"

/*
*/
CDescriptorRange::CDescriptorRange()
{
	memset(this, 0x00, sizeof(CDescriptorRange));
}

/*
*/
CDescriptorRange::CDescriptorRange(UINT count)
{
	memset(this, 0x00, sizeof(CDescriptorRange));

	m_count = count;

	m_range = new D3D12_DESCRIPTOR_RANGE1[m_count]();
}

/*
*/
CDescriptorRange::~CDescriptorRange()
{
	delete[] m_range;
}

/*
*/
void CDescriptorRange::SetRange(UINT index, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numberOfDescriptors)
{
	if (index >= m_count)
	{
		return;
	}

	m_index = index;

	m_numberOfDescriptors += numberOfDescriptors;

	m_range[m_index].RangeType = rangeType;
	m_range[m_index].NumDescriptors = numberOfDescriptors;
	m_range[m_index].BaseShaderRegister = 0;
	m_range[m_index].RegisterSpace = 0;
	m_range[m_index].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	m_range[m_index].Flags = D3D12_DESCRIPTOR_RANGE_FLAGS::D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
}