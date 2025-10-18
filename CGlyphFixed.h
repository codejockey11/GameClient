#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CGlyphVertices.h"
#include "CGraphicsAdapter.h"
#include "CShader.h"
#include "CTexture.h"
#include "CShaderBinary.h"
#include "CShaderBinaryManager.h"

class CGlyphFixed
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeapArray* m_glyphVertices;
	CLocal* m_local;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CTexture* m_glyphs;

	float m_glyphHeight;
	float m_glyphU;
	float m_glyphV;
	float m_glyphWidth;
	float m_size;
	float m_sizeRatio;

	XMFLOAT4X4 m_wvpMat;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];

	CGlyphFixed();
	CGlyphFixed(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager, CTexture* glyphs, int glyphWidth, int size);
	~CGlyphFixed();

	void Record();
};