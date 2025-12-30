#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CDescriptorRange.h"
#include "CGraphicsAdapter.h"
#include "CStaticSampler.h"

class CRootSignature
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12RootSignature> m_signature;
	ComPtr<ID3DBlob> m_errors;
	ComPtr<ID3DBlob> m_tempSignature;

	CStaticSampler m_staticSampler;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE m_featureData;
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC m_desc;

	HRESULT m_hr;

	CRootSignature();
	CRootSignature(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CRootSignature();

	void Create(int32_t count, D3D12_ROOT_PARAMETER1* pParameters);
};