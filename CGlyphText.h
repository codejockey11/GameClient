#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CGlyphTextResource.h"
#include "CGlyphVertices.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"

class CGlyphText
{
public:

	CErrorLog* m_errorLog;
	CGlyphTextResource* m_glyphTextResource;
	CGlyphVertices* m_glyphVertice;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeapArray* m_glyphVertices;
	CIndexBuffer* m_indexBuffer;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CString* m_text;
	CVec2f m_charPosition;
	CVec2f m_position;

	float m_glyphSize;
	float m_size;
	float m_sizeRatio;

	int m_charIndex;
	int m_index;

	CGlyphText();
	CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, ComPtr<ID3D12GraphicsCommandList> commandList, CHeapArray* glyphVertices, const char* text,
		CVec2f position, float size, float glyphSize);
	~CGlyphText();

	void Record();
	void UpdatePosition(CGlyphTextResource* glyphTextResource);
	void UpdateResourceForText(CGlyphTextResource* glyphTextResource, int glyphIndex);
};