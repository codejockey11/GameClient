#pragma once

#include "framework.h"

#include "../GameCommon/CNetwork.h"
#include "../GameCommon/CVec2f.h"

#include "CGraphicsAdapter.h"
#include "CKeyMap.h"

class CMouseDevice
{
public:

	enum
	{
		E_LMB = 0,
		E_MMB,
		E_RMB,
		E_SCR,

		E_KR_ONCE = 0,
		E_KR_REPEATS = 1,

		E_BUTTON_COUNT = 4
	};

	struct s_depth
	{
		long m_lX;
		long m_lY;
		long m_lZ;

		POINT m_position;
		POINT m_prevPosition;
	};

	struct s_hud
	{
		float m_lX;
		float m_lY;

		CVec2f m_position;
		CVec2f m_prevPosition;
		CVec2f m_savePosition;
	};

	bool m_positionSaved;

	BYTE m_rgbButtons[CMouseDevice::E_BUTTON_COUNT];

	CGraphicsAdapter* m_graphicsAdapter;
	CKeyMap* m_keyMap;

	float m_sensitivity;

	s_depth m_depth;

	s_hud m_hud;
	
	CMouseDevice();
	CMouseDevice(CGraphicsAdapter* graphicsAdapter, float sensitivity);
	~CMouseDevice();

	void GetState();
	void MouseClick(int32_t button, int32_t value);
	void MouseMove();
	void MouseMove(LPARAM lParam);
	void MouseWheel(long value);
	void ResetDrag();
	void ResetFirstPerson();
	void SaveLastPosition();
	void SetKeyMap(int32_t number, BYTE repeats, BYTE audience, BYTE type, BYTE data, int32_t length);
	void SetLastPosition();
	void UpdateDrag();
};