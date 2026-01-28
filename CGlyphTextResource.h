#pragma once

#include "framework.h"

#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CVertexBuffer.h"

class CGlyphTextResource
{
public:

	float m_width;

	CVertexT m_vertices[4];
	CVertexBuffer* m_vertexBuffer;
	CVec2f m_position;

	CGlyphTextResource();
	~CGlyphTextResource();
};