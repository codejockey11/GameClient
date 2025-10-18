#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CGlyphVertices.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CVertexBuffer.h"

class CGlyphText
{
public:

	class CGlyphTextResource
	{
	public:

		float m_width;
		CVec2f m_position;
		CVertexT m_vertices[4];
		CVertexBuffer* m_vertexBuffer;

		CGlyphTextResource() { memset(this, 0x00, sizeof(CGlyphTextResource)); }
		~CGlyphTextResource() { delete m_vertexBuffer; }
	};

	DWORD m_index[6];

	float m_glyphSize;
	float m_size;
	float m_sizeRatio;

	CErrorLog* m_errorLog;
	CGlyphTextResource* m_glyphTextResource;
	CGraphicsAdapter* m_graphicsAdapter;
	CIndexBuffer* m_indexBuffer;
	CString* m_text;
	CVec2f m_charPosition;
	CVec2f m_position;
	CHeapArray* m_glyphVertices;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CGlyphText();
	CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, ComPtr<ID3D12GraphicsCommandList> commandList, CHeapArray* glyphVertices, const char* text, CVec2f position, float size, float glyphSize);
	~CGlyphText();

	void Record();
	void UpdatePosition(CGlyphTextResource* glyphTextResource);
	void UpdateResourceForText(CGlyphTextResource* glyphTextResource, int glyphIndex);
};