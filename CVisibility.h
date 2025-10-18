#pragma once

#include "framework.h"

#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CSector.h"
#include "../GameCommon/CVec3f.h"
#include "../GameCommon/CVec3i.h"

#include "CObject.h"

class CVisibility
{
public:

	CHeapArray* m_collectables;
	CHeapArray* m_statics;

	CSector* m_sector;

	CVisibility();
	CVisibility(UINT width, UINT height, UINT vertical, UINT gridSize);
	~CVisibility();
};