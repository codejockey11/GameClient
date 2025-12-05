#pragma once

#include "framework.h"

#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CSector.h"

#include "CObject.h"

class CVisibility
{
public:

	CHeapArray* m_collectables;
	CHeapArray* m_statics;
	CList* m_list;
	CListNode* m_node;
	CObject* m_object;
	CSector* m_sector;

	CVisibility();
	CVisibility(int32_t width, int32_t depth, int32_t height, int32_t gridSize);
	~CVisibility();
};