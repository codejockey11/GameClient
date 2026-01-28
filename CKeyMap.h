#pragma once

#include "framework.h"

#include "../GameCommon/CNetwork.h"

class CKeyMap
{
public:

	BYTE m_repeats;
	
	CNetwork* m_network;
	
	int32_t m_count;

	CKeyMap();
	~CKeyMap();
};