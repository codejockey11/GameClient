#include "CStaticSampler.h"

/*
*/
CStaticSampler::CStaticSampler()
{
	memset(this, 0x00, sizeof(CStaticSampler));
}

/*
*/
CStaticSampler::~CStaticSampler()
{

}

/*
*/
void CStaticSampler::Add(D3D12_STATIC_SAMPLER_DESC desc)
{
	m_sampler[m_count] = desc;

	m_count++;
}

/*
*/
void CStaticSampler::Default()
{
	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ShaderRegister = 0;
	m_samplerDesc.RegisterSpace = 0;

	CStaticSampler::Add(m_samplerDesc);

	memset(&m_samplerDesc, 0x00, sizeof(D3D12_STATIC_SAMPLER_DESC));

	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_POINT;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ShaderRegister = 1;
	m_samplerDesc.RegisterSpace = 0;

	CStaticSampler::Add(m_samplerDesc);

	memset(&m_samplerDesc, 0x00, sizeof(D3D12_STATIC_SAMPLER_DESC));

	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ShaderRegister = 2;
	m_samplerDesc.RegisterSpace = 0;

	CStaticSampler::Add(m_samplerDesc);

	memset(&m_samplerDesc, 0x00, sizeof(D3D12_STATIC_SAMPLER_DESC));

	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderRegister = 3;
	m_samplerDesc.RegisterSpace = 0;

	CStaticSampler::Add(m_samplerDesc);

	memset(&m_samplerDesc, 0x00, sizeof(D3D12_STATIC_SAMPLER_DESC));

	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_ANISOTROPIC;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderRegister = 4;
	m_samplerDesc.RegisterSpace = 0;
	m_samplerDesc.MaxAnisotropy = 4;

	CStaticSampler::Add(m_samplerDesc);

	memset(&m_samplerDesc, 0x00, sizeof(D3D12_STATIC_SAMPLER_DESC));

	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_ANISOTROPIC;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderRegister = 5;
	m_samplerDesc.RegisterSpace = 0;

	CStaticSampler::Add(m_samplerDesc);

	memset(&m_samplerDesc, 0x00, sizeof(D3D12_STATIC_SAMPLER_DESC));

	m_samplerDesc.Filter = D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.ShaderRegister = 6;
	m_samplerDesc.RegisterSpace = 0;

	CStaticSampler::Add(m_samplerDesc);
}