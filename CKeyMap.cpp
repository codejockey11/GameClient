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
	if (m_network)
	{
		delete m_network;
	}
}