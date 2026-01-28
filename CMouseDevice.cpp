#include "CMouseDevice.h"

/*
*/
CMouseDevice::CMouseDevice()
{
	memset(this, 0x00, sizeof(CMouseDevice));
}

/*
*/
CMouseDevice::CMouseDevice(CGraphicsAdapter* graphicsAdapter, float sensitivity)
{
	memset(this, 0x00, sizeof(CMouseDevice));

	ShowCursor(false);

	m_graphicsAdapter = graphicsAdapter;

	m_sensitivity = sensitivity;

	SetCursorPos(m_graphicsAdapter->m_middle.x, m_graphicsAdapter->m_middle.y);

	m_hud.m_position.m_p.x = (float)m_graphicsAdapter->m_middle.x;
	m_hud.m_position.m_p.y = (float)m_graphicsAdapter->m_middle.y;

	m_keyMap = new CKeyMap[CMouseDevice::E_BUTTON_COUNT]();

	CMouseDevice::SetKeyMap(CMouseDevice::E_LMB, CMouseDevice::E_KR_REPEATS, CNetwork::ClientEvent::E_CE_TO_SERVER, CNetwork::ClientActivity::E_CA_ATTACK, 0, 1);
}

/*
*/
CMouseDevice::~CMouseDevice()
{
	SAFE_DELETE_ARRAY(m_keyMap);
}

/*
*/
void CMouseDevice::GetState()
{
	for (int32_t i = 0; i < CMouseDevice::E_BUTTON_COUNT; i++)
	{
		if (m_rgbButtons[i] > 0)
		{
			m_keyMap[i].m_count++;
		}
		else
		{
			m_keyMap[i].m_count = 0;
		}
	}
}

/*
*/
void CMouseDevice::MouseClick(int32_t button, int32_t value)
{
	m_rgbButtons[button] = (BYTE)value;
}

/*
*/
void CMouseDevice::MouseMove()
{
	GetCursorPos(&m_depth.m_position);

	m_depth.m_lX = m_depth.m_position.x - m_depth.m_prevPosition.x;
	m_depth.m_lY = m_depth.m_position.y - m_depth.m_prevPosition.y;
}

/*
*/
void CMouseDevice::MouseMove(LPARAM lParam)
{
	m_hud.m_position.m_p.x = (float)GET_X_LPARAM(lParam);
	m_hud.m_position.m_p.y = (float)GET_Y_LPARAM(lParam);
}

/*
*/
void CMouseDevice::MouseWheel(long value)
{
	m_depth.m_lZ = value;
}

/*
*/
void CMouseDevice::ResetDrag()
{
	m_hud.m_lX = 0;
	m_hud.m_lY = 0;

	m_hud.m_prevPosition.m_p.x = m_hud.m_position.m_p.x;
	m_hud.m_prevPosition.m_p.y = m_hud.m_position.m_p.y;
}

/*
*/
void CMouseDevice::ResetFirstPerson()
{
	m_depth.m_lX = 0;
	m_depth.m_lY = 0;
	m_depth.m_lZ = 0;

	SetCursorPos(m_graphicsAdapter->m_middle.x, m_graphicsAdapter->m_middle.y);

	m_depth.m_prevPosition.x = m_graphicsAdapter->m_middle.x;
	m_depth.m_prevPosition.y = m_graphicsAdapter->m_middle.y;
}

/*
*/
void CMouseDevice::SaveLastPosition()
{
	if (m_positionSaved)
	{
		return;
	}

	m_hud.m_savePosition = m_hud.m_position;

	m_positionSaved = true;

	ShowCursor(false);
}

/*
*/
void CMouseDevice::SetKeyMap(int32_t number, BYTE repeats, BYTE audience, BYTE type, BYTE data, int32_t length)
{
	m_keyMap[number].m_count = 0;
	m_keyMap[number].m_repeats = repeats;

	m_keyMap[number].m_network = new CNetwork(audience, type, nullptr, length, (void*)&data, length);
}

/*
*/
void CMouseDevice::SetLastPosition()
{
	m_hud.m_position = m_hud.m_savePosition;

	m_positionSaved = false;

	ShowCursor(true);
}

/*
*/
void CMouseDevice::UpdateDrag()
{
	m_hud.m_lX = m_hud.m_position.m_p.x - m_hud.m_prevPosition.m_p.x;
	m_hud.m_lY = m_hud.m_position.m_p.y - m_hud.m_prevPosition.m_p.y;

	m_hud.m_prevPosition.m_p.x = m_hud.m_position.m_p.x;
	m_hud.m_prevPosition.m_p.y = m_hud.m_position.m_p.y;
}