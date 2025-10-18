#pragma once

#include "framework.h"

#include "../GameCommon/CVec3f.h"
#include "../GameCommon/CVec2f.h"

class CBoundBox
{
public:

	CVec3f m_maxs;
	CVec3f m_mins;

	CBoundBox();
	CBoundBox(CBoundBox* boundBox);
	CBoundBox(CVec3f* mins, CVec3f* maxs);
	~CBoundBox();

	bool CheckPointInBox(CVec3f* point);
	bool CheckPointInBox(CVec2f* point);
	void Copy(CBoundBox* boundBox);
	void Update(CVec3f* mins, CVec3f* maxs);
};