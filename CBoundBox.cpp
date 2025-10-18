#include "CBoundBox.h"

/*
*/
CBoundBox::CBoundBox()
{
	memset(this, 0x00, sizeof(CBoundBox));
}

/*
*/
CBoundBox::CBoundBox(CBoundBox* boundBox)
{
	memset(this, 0x00, sizeof(CBoundBox));

	m_mins.m_p.x = boundBox->m_mins.m_p.x;
	m_mins.m_p.y = boundBox->m_mins.m_p.y;
	m_mins.m_p.z = boundBox->m_mins.m_p.z;

	m_maxs.m_p.x = boundBox->m_maxs.m_p.x;
	m_maxs.m_p.y = boundBox->m_maxs.m_p.y;
	m_maxs.m_p.z = boundBox->m_maxs.m_p.z;
}

/*
*/
CBoundBox::CBoundBox(CVec3f* mins, CVec3f* maxs)
{
	memset(this, 0x00, sizeof(CBoundBox));

	m_mins.m_p.x = mins->m_p.x;
	m_mins.m_p.y = mins->m_p.y;
	m_mins.m_p.z = mins->m_p.z;

	m_maxs.m_p.x = maxs->m_p.x;
	m_maxs.m_p.y = maxs->m_p.y;
	m_maxs.m_p.z = maxs->m_p.z;
}

/*
*/
CBoundBox::~CBoundBox()
{
}

/*
*/
bool CBoundBox::CheckPointInBox(CVec3f* point)
{
	if ((point->m_p.x >= m_mins.m_p.x) && (point->m_p.x <= m_maxs.m_p.x))
	{
		if ((point->m_p.y >= m_mins.m_p.y) && (point->m_p.y <= m_maxs.m_p.y))
		{
			if ((point->m_p.z >= m_mins.m_p.z) && (point->m_p.z <= m_maxs.m_p.z))
			{
				return true;
			}
		}
	}

	return false;
}

/*
*/
bool CBoundBox::CheckPointInBox(CVec2f* point)
{
	if ((point->m_p.x >= m_mins.m_p.x) && (point->m_p.x <= m_maxs.m_p.x))
	{
		if ((point->m_p.y >= m_mins.m_p.y) && (point->m_p.y <= m_maxs.m_p.y))
		{
			return true;
		}
	}

	return false;
}

/*
*/
void CBoundBox::Copy(CBoundBox* boundBox)
{
	m_mins.m_p.x = boundBox->m_mins.m_p.x;
	m_mins.m_p.y = boundBox->m_mins.m_p.y;
	m_mins.m_p.z = boundBox->m_mins.m_p.z;

	m_maxs.m_p.x = boundBox->m_maxs.m_p.x;
	m_maxs.m_p.y = boundBox->m_maxs.m_p.y;
	m_maxs.m_p.z = boundBox->m_maxs.m_p.z;
}

/*
*/
void CBoundBox::Update(CVec3f* mins, CVec3f* maxs)
{
	m_mins.m_p.x = mins->m_p.x;
	m_mins.m_p.y = mins->m_p.y;
	m_mins.m_p.z = mins->m_p.z;

	m_maxs.m_p.x = maxs->m_p.x;
	m_maxs.m_p.y = maxs->m_p.y;
	m_maxs.m_p.z = maxs->m_p.z;
}