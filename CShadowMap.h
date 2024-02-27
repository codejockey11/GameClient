#pragma once

#include "framework.h"

#include "CDepthBuffer.h"
#include "CErrorLog.h"
#include "CMaterial.h"
#include "CPipelineState.h"
#include "CRootSignature.h"
#include "CShaderHeap.h"
#include "CShaderManager.h"
#include "CTexture.h"
#include "CVideoDevice.h"

class CShadowMap
{
public:

	CDepthBuffer* m_depthBuffer;
	CPipelineState* m_pipelineState;
	CShader* m_pixelShader;
	CShader* m_vertexShader;
	CTexture* m_shadowMap;
	CVideoDevice* m_videoDevice;

	bool m_firstRecord;

	CShadowMap();
	CShadowMap(CErrorLog* errorLog, CVideoDevice* videoDevice, CShaderManager* shaderManager, CRootSignature* rootSignature, CShaderHeap* shaderHeap, int slot);
	~CShadowMap();

	void PreRecord(ComPtr<ID3D12GraphicsCommandList> commandList);
	void PostRecord(ComPtr<ID3D12GraphicsCommandList> commandList);

private:

	CErrorLog* m_errorLog;

};