#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
#include "CGlyphFixed.h"
#include "CGlyphTextResource.h"
#include "CGlyphVariable.h"
#include "CGlyphVertices.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CPipelineState.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"

class CGlyphText
{
public:

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_constantBuffer;
	CErrorLog* m_errorLog;
	CGlyphFixed* m_glyphFixed;
	CGlyphTextResource* m_glyphTextResource;
	CGlyphVariable* m_glyphVariable;
	CGlyphVertices* m_glyphVertice;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeapArray* m_glyphVertices;
	CIndexBuffer* m_indexBuffer;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CPipelineState* m_pipelineState;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CString* m_text;
	CTexture* m_glyphs;
	CVec2f m_charPosition;
	CVec2f m_position;

	float m_glyphSize;
	float m_size;
	float m_sizeRatio;

	int32_t m_bundle;
	int32_t m_charIndex;
	int32_t m_index;

	CGlyphText();
	CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CGlyphFixed* glyphFixed, int32_t bundle,
		const char* text, CVec2f position, float size);
	CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CGlyphVariable* glyphVariable, int32_t bundle,
		const char* text, CVec2f position, float size);
	~CGlyphText();

	void AllocateResources();
	void Record();
	void SetText(const char* text);
	void UpdatePosition(CGlyphTextResource* glyphTextResource);
	void UpdateResourceForText(CGlyphTextResource* glyphTextResource, int32_t glyphIndex);
};