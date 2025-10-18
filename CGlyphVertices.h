#pragma once

#include "framework.h"

#include "../GameCommon/CVertexT.h"

class CGlyphVertices
{
public:

	float m_width;
	CVertexT m_vertices[4];

	CGlyphVertices();
	~CGlyphVertices();
};