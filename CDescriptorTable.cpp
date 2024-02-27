#include "CDescriptorTable.h"

CDescriptorTable::CDescriptorTable()
{
	memset(this, 0x00, sizeof(CDescriptorTable));
}

CDescriptorTable::~CDescriptorTable()
{

}

void CDescriptorTable::SetRange(UINT index, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT count)
{
	if (index >= CDescriptorTable::MAX_RANGE)
	{
		return;
	}

	m_index = index;

	//m_descriptorTable[m_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//m_descriptorTable[m_index].ShaderVisibility = visibility;
}