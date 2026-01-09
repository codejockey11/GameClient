#pragma once

#include "framework.h"

#include "../GameCommon/CVertexT.h"

class CGlyphVertices
{
public:

	CVertexT m_vertices[4];

	float m_width;

	CGlyphVertices();
	~CGlyphVertices();
};