#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CCommandAllocator.h"
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
	CCommandAllocator* m_commandAllocator;
	CDepthBuffer* m_depthBuffers[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CIndexBuffer* m_indexBuffer;
	CLocal* m_local;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CRenderTarget* m_renderTargets[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CTexture* m_texture;
	CVec2f m_position;
	CVec2f m_size;
	CVertexBuffer* m_vertexBuffer;
	CVertexT m_vertices[4];
	CViewport* m_viewport;

	int m_bundle;

	COverhead();
	COverhead(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager, int bundle, CVec2f position, CVec2f size);
	~COverhead();

	void ClearTargets(ComPtr<ID3D12GraphicsCommandList> commandList);
	D3D12_CPU_DESCRIPTOR_HANDLE* GetDepthBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTarget();
	D3D12_RECT* GetScissorRect();
	D3D12_VIEWPORT* GetViewport();
	void Record();
	void SetCameraPosition(XMFLOAT3 position);
	void UpdatePosition();
};