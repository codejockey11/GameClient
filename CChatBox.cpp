#include "CChatBox.h"

/*
*/
CChatBox::CChatBox()
{
	memset(this, 0x00, sizeof(CChatBox));
}

/*
*/
CChatBox::CChatBox(CVideoDevice* videoDevice, CErrorLog* errorLog, CTexture* texture, CFont* font, CColor* color, CShaderManager* shaderManager, float width, float height)
{
	memset(this, 0x00, sizeof(CChatBox));

	m_videoDevice = videoDevice;

	m_errorLog = errorLog;

	m_texture = texture;

	m_font = font;

	m_color = color;

	m_shaderManager = shaderManager;

	m_isVisible = false;


	m_message = new CString(CChatBox::E_ENTRY_LENGTH);
	
	m_entries = new CLinkList<CString>();


	m_position.p.x = (float)(m_videoDevice->m_width / 2) - (width / 2.0f);
	m_position.p.y = (float)(m_videoDevice->m_height / 2) - (height / 2.0f);

	m_topLeft.p.x = m_position.p.x;
	m_topLeft.p.y = m_position.p.y;
	m_bottomRight.p.x = m_position.p.x + width;
	m_bottomRight.p.y = m_position.p.y + height;

	m_size.p.x = m_bottomRight.p.x - m_topLeft.p.x + 1.0f;
	m_size.p.y = m_bottomRight.p.y - m_topLeft.p.y + 1.0f;


	m_background = new CImage(m_videoDevice, m_errorLog, m_texture, m_shaderManager, m_size, m_position);

	m_background->m_floats->m_values[3] = 1.0f;
}

/*
*/
CChatBox::~CChatBox()
{
	delete m_background;
	delete m_entries;
	delete m_message;
}

/*
*/
void CChatBox::AddChar(const char c)
{
	if (m_cursorLocation == m_message->GetLength())
	{
		m_cursorLocation = m_message->GetLength() - 1;

		if (m_cursorLocation < 0)
		{
			m_cursorLocation = 0;
		}
	}

	m_message->GetText()[m_cursorLocation] = (char)c;

	m_cursorLocation++;
}

/*
*/
void CChatBox::AddEventMessage(const char* message)
{
	CString* chatMessage = new CString(message);

	m_entries->Add(chatMessage, m_entryNbr);

	m_entryNbr++;
}

/*
*/
void CChatBox::Backspace()
{
	m_cursorLocation--;

	if (m_cursorLocation < 0)
	{
		m_cursorLocation = 0;
	}

	m_message->GetText()[m_cursorLocation] = 0x00;
}

/*
*/
void CChatBox::DisplayConsole()
{
	m_background->Draw();
}

/*
*/
void CChatBox::DisplayText()
{
	m_lines = (int)((m_bottomRight.p.y - m_topLeft.p.y) / m_font->m_height) - 1;

	m_start = (int)(m_bottomRight.p.y - m_font->m_height - (m_font->m_height * m_entries->m_count));

	if (m_start < m_topLeft.p.y)
	{
		m_start = (int)(m_bottomRight.p.y - m_font->m_height - (m_font->m_height * m_lines));
	}


	CLinkListNode<CString>* entries = m_entries->m_list;

	if (m_entries->m_count > m_lines)
	{
		entries = m_entries->Search(m_entries->m_count - m_lines + m_scroll);
	}
	else
	{
		entries = m_entries->m_list;
	}

	while (entries->m_object)
	{
		CVertex2 position(m_topLeft.p.x + 4, (float)m_start);

		m_font->Draw(entries->m_object->GetWText(), position, m_size, m_color->SunYellow);

		m_start += (int)m_font->m_height;

		if (m_start > (m_bottomRight.p.y - m_font->m_height - m_font->m_height))
		{
			break;
		}

		entries = entries->m_next;
	}


	CVertex2 position(m_topLeft.p.x + 4, m_bottomRight.p.y - m_font->m_height);

	m_font->Draw(m_message->GetWText(), position, m_size, m_color->SunYellow);
}

/*
*/
void CChatBox::Paging(char direction)
{
	if (direction == CChatBox::E_PAGE_BACKWARD)
	{
		m_scroll--;
	}

	if ((m_entries->m_count - m_lines + m_scroll) < 0)
	{
		m_scroll++;
	}

	if (direction == CChatBox::E_PAGE_FORWARD)
	{
		m_scroll++;
	}

	if (m_scroll > 0)
	{
		m_scroll = 0;
	}
}

/*
*/
void CChatBox::Reset()
{
	m_cursorLocation = 0;

	m_message->Clear();
}

/*
*/
void CChatBox::ToggleVisibility()
{
	m_isVisible = !m_isVisible;
}