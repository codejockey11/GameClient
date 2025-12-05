#include "CRootParameter.h"

/*
*/
CRootParameter::CRootParameter()
{
	memset(this, 0x00, sizeof(CRootParameter));
}

/*
*/
CRootParameter::CRootParameter(int32_t count)
{
	memset(this, 0x00, sizeof(CRootParameter));

	m_count = count;

	m_rootParameter = new D3D12_ROOT_PARAMETER1[m_count]();
}

/*
*/
CRootParameter::~CRootParameter()
{
	SAFE_DELETE_ARRAY(m_rootParameter);
}

/*
*/
void CRootParameter::Init32BitConstant(int32_t index, int32_t count)
{
	if (index >= m_count)
	{
		return;
	}

	m_rootParameter[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	m_rootParameter[index].Constants.Num32BitValues = count;
	m_rootParameter[index].Constants.ShaderRegister = 0;
	m_rootParameter[index].Constants.RegisterSpace = 0;
	m_rootParameter[index].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}

/*
*/
void CRootParameter::InitDescriptorTable(int32_t index, int32_t count, D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges)
{
	if (index >= m_count)
	{
		return;
	}

	m_rootParameter[index].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	m_rootParameter[index].DescriptorTable.NumDescriptorRanges = count;
	m_rootParameter[index].DescriptorTable.pDescriptorRanges = pDescriptorRanges;
	m_rootParameter[index].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}