#pragma once

#include "framework.h"

#include "../GameCommon/CNetwork.h"

#include "CKeyMap.h"

class CKeyboardDevice
{
public:

	enum
	{
		E_KR_ONCE = 1,
		E_KR_REPEATS,

		E_MAX_KEYS = 256,

		// Undefined virtual keys equate to unshifted ASCII value
		E_VK_A = 65,
		E_VK_D = 68,
		E_VK_S = 83,
		E_VK_W = 87,
		E_VK_EQUALS = 187
	};

	CKeyMap* m_keyMap;

	char m_keys[CKeyboardDevice::E_MAX_KEYS];

	CKeyboardDevice();
	~CKeyboardDevice();

	void GetState();
	void KeyDown(int key);
	void KeyUp(int key);
	void SetKeyMap(UINT number, BYTE repeats, BYTE audience, BYTE type, BYTE data, int length);
};