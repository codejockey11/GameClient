#pragma once

#include "framework.h"

#include "../GameCommon/CHeapArray.h"

#include "CGlyphVertices.h"
#include "CTexture.h"

class CGlyphFixed
{
public:

	CGlyphVertices* m_glyphVertice;
	CHeapArray* m_glyphVertices;
	CTexture* m_glyphs;

	float m_glyphHeight;
	float m_glyphU;
	float m_glyphV;
	float m_glyphWidth;
	float m_size;
	float m_sizeRatio;

	CGlyphFixed();
	CGlyphFixed(CTexture* glyphs, int32_t glyphWidth, int32_t size);
	~CGlyphFixed();
};