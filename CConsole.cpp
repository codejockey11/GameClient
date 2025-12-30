#include "CConsole.h"

/*
*/
CConsole::CConsole()
{
	memset(this, 0x00, sizeof(CConsole));
}

/*
*/
CConsole::CConsole(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, int32_t listGroup, CTexture* texture,
	CFont* font, int32_t width, int32_t height)
{
	memset(this, 0x00, sizeof(CConsole));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_texture = texture;

	m_font = font;

	m_isVisible = false;

	m_message = new CString(CConsole::E_ENTRY_LENGTH);

	m_entries = new CList();

	m_position.m_p.x = (float)(m_graphicsAdapter->m_width / 2) - (width / 2.0f);
	m_position.m_p.y = (float)(m_graphicsAdapter->m_height / 2) - (height / 2.0f);

	m_topLeft.m_p.x = m_position.m_p.x;
	m_topLeft.m_p.y = m_position.m_p.y;

	m_bottomRight.m_p.x = m_position.m_p.x + width;
	m_bottomRight.m_p.y = m_position.m_p.y + height;

	m_size.m_p.x = m_bottomRight.m_p.x - m_topLeft.m_p.x + 1.0f;
	m_size.m_p.y = m_bottomRight.m_p.y - m_topLeft.m_p.y + 1.0f;

	m_background = new CImage(m_graphicsAdapter, m_errorLog, imageWvp, pipelineState, listGroup, m_texture, m_position, m_size);

	m_background->m_shaderMaterial->SetKa(1.0f, 1.0f, 1.0f, 1.0f);
}

/*
*/
CConsole::~CConsole()
{
	SAFE_DELETE(m_background);

	m_node = m_entries->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_listMessage = (CString*)m_node->m_object;

		SAFE_DELETE(m_listMessage);

		m_node = m_entries->Delete(m_node);
	}

	SAFE_DELETE(m_entries);
	SAFE_DELETE(m_message);
}

/*
*/
void CConsole::AddChar(const char c)
{
	if (m_cursorLocation == m_message->m_length)
	{
		m_cursorLocation = m_message->m_length - 1;

		if (m_cursorLocation < 0)
		{
			m_cursorLocation = 0;
		}
	}

	m_message->m_text[m_cursorLocation] = (char)c;

	m_cursorLocation++;
}

/*
*/
void CConsole::AddEventMessage(const char* message)
{
	CString* chatMessage = new CString(message);

	m_entries->Append(chatMessage, m_entryNbr);

	m_entryNbr++;
}

/*
*/
void CConsole::Backspace()
{
	m_cursorLocation--;

	if (m_cursorLocation < 0)
	{
		m_cursorLocation = 0;
	}

	m_message->m_text[m_cursorLocation] = 0x00;
}

/*
*/
void CConsole::Record()
{
	m_background->Record();
}

/*
*/
void CConsole::DisplayText()
{
	m_lines = (int32_t)((m_bottomRight.m_p.y - m_topLeft.m_p.y) / m_font->m_height) - 1;

	m_start = (int32_t)(m_bottomRight.m_p.y - m_font->m_height - (m_font->m_height * m_entries->m_count));

	if (m_start < m_topLeft.m_p.y)
	{
		m_start = (int32_t)(m_bottomRight.m_p.y - m_font->m_height - (m_font->m_height * m_lines));
	}

	m_node = m_entries->m_list;

	if (m_entries->m_count > m_lines)
	{
		m_node = m_entries->Search(m_entries->m_count - m_lines + m_scroll);
	}

	while ((m_node) && (m_node->m_object))
	{
		m_listMessage = (CString*)m_node->m_object;

		m_textPosition = CVec2f(m_topLeft.m_p.x + 4, (float)m_start);

		m_font->Draw(m_listMessage->m_text, m_textPosition, m_size, m_graphicsAdapter->m_rgbColor->White);

		m_start += (int32_t)m_font->m_height;

		if (m_start > (m_bottomRight.m_p.y - m_font->m_height - m_font->m_height))
		{
			break;
		}

		m_node = m_node->m_next;
	}

	m_textPosition = CVec2f(m_topLeft.m_p.x + 4, m_bottomRight.m_p.y - m_font->m_height);

	m_font->Draw(m_message->m_text, m_textPosition, m_size, m_graphicsAdapter->m_rgbColor->White);
}

/*
*/
void CConsole::Paging(char direction)
{
	if (direction == CConsole::E_PAGE_BACKWARD)
	{
		m_scroll--;
	}

	if ((m_entries->m_count - m_lines + m_scroll) < 0)
	{
		m_scroll++;
	}

	if (direction == CConsole::E_PAGE_FORWARD)
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
void CConsole::Reset()
{
	m_cursorLocation = 0;

	m_message->Clear();
}

/*
*/
void CConsole::ToggleVisibility()
{
	m_isVisible = !m_isVisible;
}