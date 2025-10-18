#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CDescriptorRange.h"
#include "CGraphicsAdapter.h"
#include "CStaticSampler.h"

class CRootSignature
{
public:

	ComPtr<ID3DBlob> m_tempSignature;
	ComPtr<ID3DBlob> m_errors;
	ComPtr<ID3D12RootSignature> m_signature;

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CStaticSampler m_staticSampler;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC m_desc;
	D3D12_FEATURE_DATA_ROOT_SIGNATURE m_featureData;

	HRESULT m_hr;

	CRootSignature();
	CRootSignature(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CRootSignature();

	void Create(int count, D3D12_ROOT_PARAMETER1* pParameters);
};