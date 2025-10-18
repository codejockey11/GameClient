#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CDepthBuffer.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"
#include "CTexture.h"
#include "CVertexBuffer.h"
#include "CViewport.h"

class COverhead
{
public:

	CCamera* m_camera;
	CDepthBuffer* m_depthBuffers[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CLocal* m_local;
	CRenderTarget* m_renderTargets[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CTexture* m_texture;
	CVec2f m_position;
	CVec2f m_size;
	CViewport* m_viewport;

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_resources[CGraphicsAdapter::E_BACKBUFFER_COUNT];

	COverhead();
	COverhead(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager, CVec2f position, CVec2f size);
	~COverhead();

	void ClearTargets();
	void Record();
	void SetCameraPosition(XMFLOAT3 position);
	void UpdatePosition();

private:

	CErrorLog* m_errorLog;
	CIndexBuffer* m_indexBuffer;
	CVertexBuffer* m_vertexBuffer;
	CVertexT m_vertices[4];
	CGraphicsAdapter* m_graphicsAdapter;
};