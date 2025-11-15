#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CBoundBox.h"
#include "CCommandAllocator.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"
#include "CTexture.h"
#include "CVertexBuffer.h"

class CImage
{
public:

	bool m_needsUpload;

	CBoundBox* m_box;
	CCommandAllocator* m_commandAllocator;
	CGraphicsAdapter* m_graphicsAdapter;
	CIndexBuffer* m_indexBuffer;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

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

	int m_bundle;

	float m_cb;
	float m_cl;
	float m_cr;
	float m_ct;
	float m_highlight;

	CImage();
	CImage(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, int bundle, CTexture* texture, CVec2f position, CVec2f size);
	~CImage();

	void Move(float x, float y);
	void Record();
	void SetPosition(CVec2f* position);
	void UpdateBuffer();
};