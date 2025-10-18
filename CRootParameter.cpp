#include "CRootParameter.h"

/*
*/
CRootParameter::CRootParameter()
{
	memset(this, 0x00, sizeof(CRootParameter));
}

/*
*/
CRootParameter::CRootParameter(UINT count)
{
	memset(this, 0x00, sizeof(CRootParameter));

	m_count = count;

	m_rootParameter = new D3D12_ROOT_PARAMETER1[m_count]();
}

/*
*/
CRootParameter::~CRootParameter()
{
	delete[] m_rootParameter;
}

/*
*/
void CRootParameter::Init32BitConstant(UINT index, UINT count)
{
	if (index >= m_count)
	{
		return;
	}

	m_index = index;

	m_rootParameter[m_index] = {};

	m_rootParameter[m_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	m_rootParameter[m_index].Constants.Num32BitValues = count;
	m_rootParameter[m_index].Constants.ShaderRegister = 0;
	m_rootParameter[m_index].Constants.RegisterSpace = 0;
	m_rootParameter[m_index].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}

/*
*/
void CRootParameter::InitDescriptorTable(UINT index, UINT count, D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges)
{
	if (index >= m_count)
	{
		return;
	}

	m_index = index;

	m_rootParameter[m_index] = {};

	m_rootParameter[m_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameter[m_index].DescriptorTable.NumDescriptorRanges = count;
	m_rootParameter[m_index].DescriptorTable.pDescriptorRanges = pDescriptorRanges;
	m_rootParameter[m_index].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}