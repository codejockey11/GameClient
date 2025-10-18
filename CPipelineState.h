#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CVec3f.h"

#include "CGraphicsAdapter.h"
#include "CRootSignature.h"

class CPipelineState
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12PipelineState> m_pipelineState;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc;
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_computeDesc;

	CPipelineState();
	CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, BYTE type, D3D12_CULL_MODE cullMode);
	~CPipelineState();

	void Create(bool alphaToCoverageEnable, bool blendEnable, bool depthEnable, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType, CRootSignature* rootSignature);
	void CreateCompute(CRootSignature* rootSignature, ID3DBlob*);

	void CreateShadowMap(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType, CRootSignature* rootSignature);

private:

	D3D12_INPUT_ELEMENT_DESC m_vertexDesc[3];

	D3D12_RENDER_TARGET_BLEND_DESC m_blendDesc;

	UINT m_vertexDescCount;

	HRESULT m_hr;
};