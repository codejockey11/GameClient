#pragma once

#include "framework.h"

#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVec3f.h"

class CBoundBox
{
public:

	CVec3f m_max;
	CVec3f m_min;

	CBoundBox();
	CBoundBox(float x, float y, float z, float width, float height, float depth);
	~CBoundBox();

	bool CheckPointInBox(CVec3f* point);
	bool CheckPointInBox(CVec2f* point);
	void Move(float x, float y, float z);
};