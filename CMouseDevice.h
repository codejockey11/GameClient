#pragma once

#include "framework.h"

#include "CErrorLog.h"
#include "CKeyMap.h"
#include "CNetwork.h"
#include "CVertex.h"
#include "CVideoDevice.h"

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

	BYTE m_rgbButtons[CMouseDevice::E_BUTTON_COUNT];
	
	CKeyMap* m_keyMap;
	CVertex2 m_position;
	
	POINT m_physicalPosition;
	POINT m_prevPosition;
	POINT m_savePosition;
	
	bool m_positionSaved;
	
	float m_sensitivity;
	
	long m_lX;
	long m_lY;
	long m_lZ;


	CMouseDevice();
	CMouseDevice(CVideoDevice* videoDevice, CErrorLog* errorLog, float sensitivity);
	~CMouseDevice();

	void FirstPersonReset();
	void GetState();
	void MouseClick(int button, int value);
	void MouseMove();
	void MouseWheel(long value);
	void SaveLastPosition();
	void SetKeyMap(UINT number, BYTE repeats, BYTE audience, BYTE type, BYTE data, int length);
	void SetLastPosition();

private:

	CErrorLog* m_errorLog;
	CVideoDevice* m_videoDevice;
};