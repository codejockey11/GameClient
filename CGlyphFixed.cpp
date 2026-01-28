#include "CGlyphFixed.h"

/*
*/
CGlyphFixed::CGlyphFixed()
{
	memset(this, 0x00, sizeof(CGlyphFixed));
}

/*
*/
CGlyphFixed::CGlyphFixed(CTexture* glyphs, int32_t glyphWidth, int32_t size)
{
	memset(this, 0x00, sizeof(CGlyphFixed));

	m_glyphs = glyphs;

	m_glyphWidth = (float)glyphWidth;
	
	m_size = (float)size;

	m_glyphHeight = (float)m_glyphs->m_height;
	
	m_glyphU = m_glyphWidth / (float)m_glyphs->m_width;
	m_glyphV = 1.0f;

	m_sizeRatio = m_size / m_glyphHeight;

	// vertex uv draw order
	// 0---1
	// | \ |
	// 3---2

	m_glyphVertices = new CHeapArray(true, sizeof(CGlyphVertices), 1, 94);

	for (int32_t i = 0; i < 94; i++)
	{
		m_glyphVertice = (CGlyphVertices*)m_glyphVertices->GetElement(1, i);

		m_glyphVertice->m_width = m_glyphWidth;

		m_glyphVertice->m_vertices[0].m_p.x = 0.0f;
		m_glyphVertice->m_vertices[0].m_p.y = 0.0f;

		m_glyphVertice->m_vertices[1].m_p.x = m_glyphVertice->m_width * m_sizeRatio;
		m_glyphVertice->m_vertices[1].m_p.y = 0.0f;

		m_glyphVertice->m_vertices[2].m_p.x = m_glyphVertice->m_width * m_sizeRatio;
		m_glyphVertice->m_vertices[2].m_p.y = m_glyphHeight * m_sizeRatio;

		m_glyphVertice->m_vertices[3].m_p.x = 0.0f;
		m_glyphVertice->m_vertices[3].m_p.y = m_glyphHeight * m_sizeRatio;

		m_glyphVertice->m_vertices[0].m_uv.x = (float)i * m_glyphU;
		m_glyphVertice->m_vertices[0].m_uv.y = 0.0f;

		m_glyphVertice->m_vertices[1].m_uv.x = ((float)i + 1.0f) * m_glyphU;
		m_glyphVertice->m_vertices[1].m_uv.y = 0.0f;

		m_glyphVertice->m_vertices[2].m_uv.x = ((float)i + 1.0f) * m_glyphU;
		m_glyphVertice->m_vertices[2].m_uv.y = m_glyphV;

		m_glyphVertice->m_vertices[3].m_uv.x = (float)i * m_glyphU;
		m_glyphVertice->m_vertices[3].m_uv.y = m_glyphV;
	}
}

/*
*/
CGlyphFixed::~CGlyphFixed()
{
	SAFE_DELETE(m_glyphVertices);
}