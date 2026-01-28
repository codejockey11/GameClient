#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"

#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
#include "CDepthBuffer.h"
#include "CGraphicsAdapter.h"
#include "CLight.h"
#include "CPipelineState.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"
#include "CShaderHeap.h"

class CShadowMap
{
public:

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_constantBuffer;
	CDepthBuffer* m_depthBuffer;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLight* m_light;
	CLocal* m_local;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CPipelineState* m_pipelineState;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShaderHeap* m_shaderHeap;
	CVec3f m_lightPosition;
	CViewport m_viewport;

	CShadowMap();
	CShadowMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager);
	~CShadowMap();

	ID3D12Resource* GetResource();
	D3D12_SHADER_RESOURCE_VIEW_DESC* GetSRV();
	void PostRecord();
	void PreRecord();
};