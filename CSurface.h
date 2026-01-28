#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CVertexNT.h"

#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CVertexBuffer.h"

class CSurface
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CIndexBuffer* m_indexBuffer;
	CVertexBuffer* m_vertexBuffer;
	CVertexNT* m_vertices;

	int16_t* m_indices;

	int32_t m_indexCount;
	int32_t m_material;
	int32_t m_vertexCount;

	CSurface();
	CSurface(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CSurface();

	void Read(FILE* file);
};