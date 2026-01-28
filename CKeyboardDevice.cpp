#include "CKeyboardDevice.h"

/*
*/
CKeyboardDevice::CKeyboardDevice()
{
	memset(this, 0x00, sizeof(CKeyboardDevice));

	m_keyMap = new CKeyMap[CKeyboardDevice::E_MAX_KEYS]();

	CKeyboardDevice::SetKeyMap(CKeyboardDevice::E_VK_EQUALS, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_WINDOW_MODE, 0, 0);

	CKeyboardDevice::SetKeyMap(VK_F3, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CONSOLE, 0, 0);

	CKeyboardDevice::SetKeyMap(VK_F5, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_BROWSER, 0, 0);

	CKeyboardDevice::SetKeyMap(VK_F6, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_CLOSE_BROWSER, 0, 0);

	CKeyboardDevice::SetKeyMap(VK_F7, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_REQUEST_ACCOUNT_INFO, 0, 0);

	CKeyboardDevice::SetKeyMap(VK_ESCAPE, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_LOCAL, CNetwork::ClientEvent::E_CE_EXIT_GAME, 0, 0);

	CKeyboardDevice::SetKeyMap(CKeyboardDevice::E_VK_W, CKeyboardDevice::E_KR_REPEATS,
		CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ClientActivity::E_CA_FORWARD, 0, 1);
	
	CKeyboardDevice::SetKeyMap(CKeyboardDevice::E_VK_S, CKeyboardDevice::E_KR_REPEATS,
		CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ClientActivity::E_CA_BACKWARD, 0, 1);
	
	CKeyboardDevice::SetKeyMap(CKeyboardDevice::E_VK_A, CKeyboardDevice::E_KR_REPEATS,
		CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ClientActivity::E_CA_STEP_LEFT, 0, 1);
	
	CKeyboardDevice::SetKeyMap(CKeyboardDevice::E_VK_D, CKeyboardDevice::E_KR_REPEATS,
		CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ClientActivity::E_CA_STEP_RIGHT, 0, 1);
	
	CKeyboardDevice::SetKeyMap(VK_SPACE, CKeyboardDevice::E_KR_ONCE,
		CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ClientActivity::E_CA_ATTACK, 0, 1);
}

/*
*/
CKeyboardDevice::~CKeyboardDevice()
{
	SAFE_DELETE_ARRAY(m_keyMap);
}

/*
*/
void CKeyboardDevice::GetState()
{
	for (int32_t i = 0; i < CKeyboardDevice::E_MAX_KEYS; i++)
	{
		if (m_keys[i])
		{
			m_keyMap[i].m_count++;

			if (m_keyMap[i].m_count > 2)
			{
				m_keyMap[i].m_count = 2;
			}
		}
		else
		{
			m_keyMap[i].m_count = 0;
		}
	}
}

/*
*/
void CKeyboardDevice::KeyDown(int32_t key)
{
	m_keys[key] = 1;
}

/*
*/
void CKeyboardDevice::KeyUp(int32_t key)
{
	m_keys[key] = 0;
}

/*
*/
void CKeyboardDevice::SetKeyMap(int32_t key, BYTE repeats, BYTE audience, BYTE type, BYTE data, int32_t length)
{
	m_keyMap[key].m_count = 0;
	m_keyMap[key].m_repeats = repeats;

	if (m_keyMap[key].m_network)
	{
		SAFE_DELETE(m_keyMap[key].m_network);
	}

	m_keyMap[key].m_network = new CNetwork(audience, type, nullptr, 0, (void*)&data, length);
}