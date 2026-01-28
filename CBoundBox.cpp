#include "CBoundBox.h"

/*
*/
CBoundBox::CBoundBox()
{
	memset(this, 0x00, sizeof(CBoundBox));
}

/*
*/
CBoundBox::CBoundBox(float x, float y, float z, float width, float height, float depth)
{
	memset(this, 0x00, sizeof(CBoundBox));

	m_min.m_p.x = x;
	m_min.m_p.y = y;
	m_min.m_p.z = z;

	m_max.m_p.x = x + width;
	m_max.m_p.y = y + height;
	m_max.m_p.z = z + depth;
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
	if ((point->m_p.x >= m_min.m_p.x) && (point->m_p.x <= m_max.m_p.x))
	{
		if ((point->m_p.y >= m_min.m_p.y) && (point->m_p.y <= m_max.m_p.y))
		{
			if ((point->m_p.z >= m_min.m_p.z) && (point->m_p.z <= m_max.m_p.z))
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
	if ((point->m_p.x >= m_min.m_p.x) && (point->m_p.x <= m_max.m_p.x))
	{
		if ((point->m_p.y >= m_min.m_p.y) && (point->m_p.y <= m_max.m_p.y))
		{
			return true;
		}
	}

	return false;
}

/*
*/
void CBoundBox::Move(float x, float y, float z)
{
	m_min.m_p.x += x;
	m_min.m_p.y += y;
	m_min.m_p.z += z;

	m_max.m_p.x += x;
	m_max.m_p.y += y;
	m_max.m_p.z += z;
}