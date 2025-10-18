#include "CGlyphText.h"

/*
*/
CGlyphText::CGlyphText()
{
	memset(this, 0x00, sizeof(CGlyphText));
}

/*
*/
CGlyphText::CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, ComPtr<ID3D12GraphicsCommandList> commandList, CHeapArray* glyphVertices, const char* text, CVec2f position, float size, float glyphSize)
{
	memset(this, 0x00, sizeof(CGlyphText));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_commandList = commandList;

	m_glyphVertices = glyphVertices;

	m_text = new CString(text);

	m_position = position;

	m_size = (float)size;

	m_glyphSize = glyphSize;

	m_sizeRatio = m_size / m_glyphSize;

	m_glyphTextResource = new CGlyphTextResource[m_text->m_length]();

	int index = 0;

	for (int i = 0; i < m_text->m_length; i++)
	{
		int x = m_text->m_text[i] - 0x21;

		if (x >= 0)
		{
			CGlyphText::UpdateResourceForText(&m_glyphTextResource[index], x);
		}

		m_glyphTextResource[index].m_vertexBuffer = new CVertexBuffer();

		m_glyphTextResource[index].m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, VertexType::E_VT_VERTEXT, 4, (void*)m_glyphTextResource[index].m_vertices);

		m_glyphTextResource[index].m_vertexBuffer->SetCommandList(m_commandList);

		index++;
	}

	m_index[0] = 0;
	m_index[1] = 1;
	m_index[2] = 2;

	m_index[3] = 0;
	m_index[4] = 2;
	m_index[5] = 3;

	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, m_errorLog, 6, m_index);
}

/*
*/
CGlyphText::~CGlyphText()
{
	delete m_indexBuffer;
	delete[] m_glyphTextResource;
	delete m_text;
}

/*
*/
void CGlyphText::Record()
{
	m_indexBuffer->SetCommandList(m_commandList);

	m_indexBuffer->Draw();

	m_charPosition = m_position;

	int index = 0;

	for (int i = 0; i < strlen(m_text->m_text); i++)
	{
		int x = m_text->m_text[i] - 0x21;

		if (x >= 0)
		{
			m_glyphTextResource[index].m_position = m_charPosition;

			CGlyphText::UpdateResourceForText(&m_glyphTextResource[index], x);

			CGlyphText::UpdatePosition(&m_glyphTextResource[index]);

			m_glyphTextResource[index].m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);

			m_charPosition.m_p.x += (m_sizeRatio * m_glyphTextResource[index].m_width);
		}
		else
		{
			m_charPosition.m_p.x += (m_sizeRatio * 8.0f);
		}

		index++;
	}
}

/*
*/
void CGlyphText::UpdatePosition(CGlyphTextResource* glyphTextResource)
{
	glyphTextResource->m_vertices[0].m_p.x = glyphTextResource->m_position.m_p.x;
	glyphTextResource->m_vertices[1].m_p.x = glyphTextResource->m_position.m_p.x + glyphTextResource->m_width * m_sizeRatio;
	glyphTextResource->m_vertices[2].m_p.x = glyphTextResource->m_position.m_p.x + glyphTextResource->m_width * m_sizeRatio;
	glyphTextResource->m_vertices[3].m_p.x = glyphTextResource->m_position.m_p.x;

	glyphTextResource->m_vertices[0].m_p.y = glyphTextResource->m_position.m_p.y;
	glyphTextResource->m_vertices[1].m_p.y = glyphTextResource->m_position.m_p.y;
	glyphTextResource->m_vertices[2].m_p.y = glyphTextResource->m_position.m_p.y + m_glyphSize * m_sizeRatio;
	glyphTextResource->m_vertices[3].m_p.y = glyphTextResource->m_position.m_p.y + m_glyphSize * m_sizeRatio;

	glyphTextResource->m_vertexBuffer->LoadBuffer((void*)glyphTextResource->m_vertices);
}

/*
*/
void CGlyphText::UpdateResourceForText(CGlyphTextResource* glyphTextResource, int glyphIndex)
{
	CGlyphVertices* glyphVertices = (CGlyphVertices*)m_glyphVertices->GetElement(1, glyphIndex);

	glyphTextResource->m_width = glyphVertices->m_width;

	glyphTextResource->m_vertices[0].m_p.x = glyphVertices->m_vertices[0].m_p.x;
	glyphTextResource->m_vertices[0].m_p.y = glyphVertices->m_vertices[0].m_p.y;

	glyphTextResource->m_vertices[1].m_p.x = glyphVertices->m_vertices[1].m_p.x;
	glyphTextResource->m_vertices[1].m_p.y = glyphVertices->m_vertices[1].m_p.y;

	glyphTextResource->m_vertices[2].m_p.x = glyphVertices->m_vertices[2].m_p.x;
	glyphTextResource->m_vertices[2].m_p.y = glyphVertices->m_vertices[2].m_p.y;

	glyphTextResource->m_vertices[3].m_p.x = glyphVertices->m_vertices[3].m_p.x;
	glyphTextResource->m_vertices[3].m_p.y = glyphVertices->m_vertices[3].m_p.y;

	glyphTextResource->m_vertices[0].m_uv.x = glyphVertices->m_vertices[0].m_uv.x;
	glyphTextResource->m_vertices[0].m_uv.y = glyphVertices->m_vertices[0].m_uv.y;

	glyphTextResource->m_vertices[1].m_uv.x = glyphVertices->m_vertices[1].m_uv.x;
	glyphTextResource->m_vertices[1].m_uv.y = glyphVertices->m_vertices[1].m_uv.y;

	glyphTextResource->m_vertices[2].m_uv.x = glyphVertices->m_vertices[2].m_uv.x;
	glyphTextResource->m_vertices[2].m_uv.y = glyphVertices->m_vertices[2].m_uv.y;

	glyphTextResource->m_vertices[3].m_uv.x = glyphVertices->m_vertices[3].m_uv.x;
	glyphTextResource->m_vertices[3].m_uv.y = glyphVertices->m_vertices[3].m_uv.y;
}