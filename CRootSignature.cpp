#include "CRootSignature.h"

/*
*/
CRootSignature::CRootSignature()
{
	memset(this, 0x00, sizeof(CRootSignature));
}

/*
*/
CRootSignature::CRootSignature(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CRootSignature));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_staticSampler.Default();

	m_featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_1;

	m_hr = m_graphicsAdapter->m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_ROOT_SIGNATURE, &m_featureData, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE));

	if (m_hr != S_OK)
	{
		m_featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_1;
	}
}

/*
*/
CRootSignature::~CRootSignature()
{
	m_signature.Reset();
}

/*
*/
void CRootSignature::Create(int32_t count, D3D12_ROOT_PARAMETER1* pParameters)
{
	m_desc.Version = m_featureData.HighestVersion;

	m_desc.Desc_1_1.NumParameters = count;
	m_desc.Desc_1_1.pParameters = pParameters;
	m_desc.Desc_1_1.NumStaticSamplers = m_staticSampler.m_count;
	m_desc.Desc_1_1.pStaticSamplers = m_staticSampler.m_sampler;
	m_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	m_hr = D3D12SerializeVersionedRootSignature(&m_desc, &m_tempSignature, &m_errors);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CRootSignature::Create::D3D12SerializeVersionedRootSignature:", m_hr);

		if (m_errors)
		{
			m_errorLog->WriteError(true, "%s\n", m_errors->GetBufferPointer());
		}

		return;
	}

	m_hr = m_graphicsAdapter->m_device->CreateRootSignature(0, m_tempSignature->GetBufferPointer(), m_tempSignature->GetBufferSize(),
		__uuidof(ID3D12RootSignature), (void**)&m_signature);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CRootSignature::Create::CreateRootSignature:", m_hr);

		return;
	}

	m_signature->SetName(L"CRootSignature::RootSignature");

	m_errors.Reset();
}