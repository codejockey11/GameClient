#include "CKeyMap.h"

/*
*/
CKeyMap::CKeyMap()
{
	memset(this, 0x00, sizeof(CKeyMap));
}

/*
*/
CKeyMap::~CKeyMap()
{
	SAFE_DELETE(m_network);
}