#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"

#include "CDepthBuffer.h"
#include "CGraphicsAdapter.h"
#include "CLight.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"
#include "CTexture.h"

class CShadowMap
{
public:

	CLight m_light;

	CDepthBuffer* m_depthBuffers[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLocal* m_local;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CTexture* m_texture;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];

	CShadowMap();
	CShadowMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager);
	~CShadowMap();

	void PreRecord();
	void PostRecord();
};