#include "CTextInput.h"

/*
*/
CTextInput::CTextInput()
{
	memset(this, 0x00, sizeof(CTextInput));
}

/*
*/
CTextInput::CTextInput(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CFont* font, CTexture* texture,
	CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CTextInput));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_font = font;

	m_position = position;

	m_size = size;

	m_image = new CImage(m_graphicsAdapter, m_errorLog, shaderBinaryManager, texture, m_position, m_size);

	m_image->m_highlight = 0.75f;

	m_value = new CString(CString::E_LARGE);
}

/*
*/
CTextInput::~CTextInput()
{
	delete m_value;
	delete m_image;
}

/*
*/
void CTextInput::Active()
{
	m_image->m_highlight = 1.0f;

	m_isActive = true;
}

/*
*/
void CTextInput::AddChar(const char c)
{
	if (m_cursorLocation == m_value->m_length)
	{
		m_cursorLocation = m_value->m_length - 1;

		if (m_cursorLocation < 0)
		{
			m_cursorLocation = 0;
		}
	}

	m_value->m_text[m_cursorLocation] = (char)c;

	m_cursorLocation++;
}

/*
*/
void CTextInput::Backspace()
{
	m_cursorLocation--;

	if (m_cursorLocation < 0)
	{
		m_cursorLocation = 0;
	}

	m_value->m_text[m_cursorLocation] = 0x00;
}

/*
*/
void CTextInput::CheckMouseOver()
{
	if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
	{
		if (m_image->m_box->CheckPointInBox(&m_mouseDevice->m_hud.m_position))
		{
			m_isMouseOver = true;

			if (m_isActive)
			{
				CTextInput::Inactive();
			}
			else
			{
				CTextInput::Active();
			}
		}
		else
		{
			m_isMouseOver = false;

			CTextInput::Inactive();
		}
	}
}

/*
*/
void CTextInput::Delete()
{
	int i = m_cursorLocation;

	for (; i < m_value->m_length - 1; i++)
	{
		m_value->m_text[i] = m_value->m_text[i + 1];
	}

	m_value->m_text[i] = 0x00;
}

/*
*/
void CTextInput::DisplayText()
{
	m_font->Draw(m_value->m_text, m_position, m_size, m_graphicsAdapter->m_color->SunYellow);
}

/*
*/
void CTextInput::Inactive()
{
	m_image->m_highlight = 0.75f;

	m_isActive = false;
}

/*
*/
void CTextInput::Record()
{
	m_image->Record();
}

/*
*/
void CTextInput::Reset()
{
	m_cursorLocation = 0;

	m_value->Clear();
}