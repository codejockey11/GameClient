#pragma once

#include "framework.h"

#include "CNetwork.h"

class CKeyMap
{
public:

	BYTE m_repeats;
	
	CNetwork* m_network;
	
	UINT m_count;

	CKeyMap();
	~CKeyMap();
};