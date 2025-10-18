#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"
#include "CTexture.h"
#include "CVertexBuffer.h"

class CImage
{
public:

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CGraphicsAdapter* m_graphicsAdapter;
	CIndexBuffer* m_indexBuffer;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CTexture* m_texture;
	CVec2f m_cellSize;
	CVec2f m_position;
	CVec2f m_scaleSize;
	CVec2f m_sheetSize;
	CVec2f m_size;
	CVertexBuffer* m_vertexBuffer;
	CVertexT m_vertices[4];

	DWORD m_color;

	float m_cb;
	float m_cl;
	float m_cr;
	float m_ct;
	float m_highlight;

	XMFLOAT4X4 m_wvpMat;

	CImage();
	CImage(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CTexture* texture, CVec2f position, CVec2f size);
	~CImage();

	void Record();
	void UpdatePosition();
};