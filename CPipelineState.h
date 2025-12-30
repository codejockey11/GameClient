#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"
#include "CRootParameter.h"
#include "CRootSignature.h"
#include "CShaderBinary.h"

class CPipelineState
{
public:

	enum VertexType
	{
		E_VT_VERTEX = 0,
		E_VT_VERTEX2D,
		E_VT_VERTEXLINE,
		E_VT_VERTEXNT,
		E_VT_VERTEXRGBA,
		E_VT_VERTEXT
	};

	CDescriptorRange* m_descriptorRange;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	
	ComPtr<ID3D12PipelineState> m_pipelineState;
	
	CRootParameter* m_rootParameter;
	CRootSignature* m_rootSignature;

	D3D12_COMPUTE_PIPELINE_STATE_DESC m_computeDesc;
	D3D12_DEPTH_STENCILOP_DESC m_defaultStencilOp;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc;
	D3D12_INPUT_ELEMENT_DESC m_vertexDesc[3];
	D3D12_RENDER_TARGET_BLEND_DESC m_blendDesc;
	D3D12_RENDER_TARGET_BLEND_DESC m_defaultRenderTargetBlendDesc;

	HRESULT m_hr;

	int32_t m_vertexDescCount;

	CPipelineState();
	CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t constantBufferCount, int32_t textureCount);
	CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t constantBufferCount, int32_t textureCount, BYTE type, D3D12_CULL_MODE cullMode);
	~CPipelineState();

	void AllocateSignature(int32_t constantBufferCount, int32_t textureCount);
	void Create(bool alphaToCoverageEnable, bool blendEnable, bool depthEnable, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType);
	void CreateCompute();
	void CreateShadowMap(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType);
	void SetComputeShader(CShaderBinary* shaderBinary);
	void SetPixelShader(CShaderBinary* shaderBinary);
	void SetVertexShader(CShaderBinary* shaderBinary);
};