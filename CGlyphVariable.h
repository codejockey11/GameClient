#pragma once

#include "framework.h"

#include "../GameCommon/CHeapArray.h"

#include "CGlyphVertices.h"
#include "CTexture.h"

class CGlyphVariable
{
public:

	CGlyphVertices* m_glyphVertice;
	CHeapArray* m_glyphVertices;
	CTexture* m_glyphs;
	
	float m_glyphHeight;
	float m_glyphV;
	float m_glyphWidth[95];
	float m_size;
	float m_sizeRatio;

	CGlyphVariable();
	CGlyphVariable(CTexture* glyphs, int32_t size);
	~CGlyphVariable();

	void DefineUForResourceUV();
};