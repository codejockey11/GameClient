#include "CButton.h"

/*
*/
CButton::CButton()
{
	memset(this, 0x00, sizeof(CButton));
}

/*
*/
CButton::CButton(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, const char* name, CFont* font,
	CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image, CVec2f position, CVec2f size, BYTE audience, BYTE type)
{
	memset(this, 0x00, sizeof(CButton));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_position = position;

	m_size = size;

	m_name = new CString(name);

	CVec3f v1(m_position.m_p.x, m_position.m_p.y, 0.0f);
	CVec3f v2(m_position.m_p.x + m_size.m_p.x, m_position.m_p.y + m_size.m_p.y, 0.0f);

	m_box = new CBoundBox(&v1, &v2);

	m_image = new CImage(graphicsAdapter, errorLog, shaderBinaryManager, image, position, size);

	m_font = font;

	m_network = new CNetwork(audience, type, nullptr, 0, nullptr, 0);

	m_soundOff = new CSound(errorLog, soundDevice, soundOff, false);
	m_soundOn = new CSound(errorLog, soundDevice, soundOn, false);
	m_soundClicked = new CSound(errorLog, soundDevice, soundClicked, false);

	m_soundOffPlayed = true;
	m_soundOnPlayed = false;
	m_isMouseOver = false;
	m_isInitialized = true;
	
	m_image->m_highlight = 0.75f;
}

/*
*/
CButton::~CButton()
{
	delete m_box;
	delete m_soundClicked;
	delete m_soundOn;
	delete m_soundOff;
	delete m_network;
	delete m_image;
	delete m_name;
}

/*
*/
void CButton::Active()
{
	m_image->m_highlight = 1.0f;

	m_isMouseOver = true;

	if (m_soundOnPlayed == false)
	{
		m_soundOffPlayed = false;

		m_soundOnPlayed = true;

		m_soundOn->StartSound();
	}
}

/*
*/
void CButton::CheckMouseOver()
{
	if (m_box->CheckPointInBox(&m_mouseDevice->m_hud.m_position))
	{
		CButton::Active();
	}
	else
	{
		CButton::Inactive();
	}
}

/*
*/
void CButton::DisplayText()
{
	SIZE s = m_font->TextDimensions(m_name->m_text);

	CVec2f position(m_image->m_position.m_p.x + (m_image->m_size.m_p.x / 2.0f) - (s.cx / 2.0f),
		m_image->m_position.m_p.y + (m_image->m_size.m_p.y / 2.0f) - (s.cy / 2.0f));

	if (m_isMouseOver)
	{
		m_font->Draw(m_name->m_text, position, m_image->m_size, m_graphicsAdapter->m_color->SunYellow);
	}
	else
	{
		m_font->Draw(m_name->m_text, position, m_image->m_size, m_graphicsAdapter->m_color->ZombieGreen);
	}
}

/*
*/
void CButton::Inactive()
{
	m_image->m_highlight = 0.75f;

	m_isMouseOver = false;

	if (m_soundOffPlayed == false)
	{
		m_soundOffPlayed = true;

		m_soundOnPlayed = false;

		//m_soundOff->StartSound();
	}
}

/*
*/
void CButton::Record()
{
	m_image->Record();
}