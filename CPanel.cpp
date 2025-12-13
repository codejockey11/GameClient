#include "CPanel.h"

/*
*/
CPanel::CPanel()
{
	memset(this, 0x00, sizeof(CPanel));
}

/*
*/
CPanel::CPanel(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CPipelineState* pipelineState, int32_t listGroup, CImage* cursor,
	CTexture* texture, CFont* font,	const char* title, CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CPanel));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_cursor = cursor;

	m_font = font;

	m_background = new CImage(m_graphicsAdapter, m_errorLog, pipelineState, listGroup, texture, position, size);

	m_title = new CString(title);

	m_buttons = new CList();

	m_images = new CList();

	m_textInputs = new CList();
}

/*
*/
CPanel::~CPanel()
{
	m_node = m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_textInput = (CTextInput*)m_node->m_object;

		SAFE_DELETE(m_textInput);

		m_node = m_textInputs->Delete(m_node);
	}

	SAFE_DELETE(m_textInputs);

	m_node = m_images->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_image = (CImage*)m_node->m_object;

		SAFE_DELETE(m_image);

		m_node = m_images->Delete(m_node);
	}

	SAFE_DELETE(m_images);

	m_node = m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_button = (CButton*)m_node->m_object;

		SAFE_DELETE(m_button);

		m_node = m_buttons->Delete(m_node);
	}

	SAFE_DELETE(m_buttons);

	SAFE_DELETE(m_title);
	SAFE_DELETE(m_background);
}

/*
*/
CNetwork* CPanel::Active()
{
	m_isMouseOver = true;

	m_background->m_highlight = 1.0f;

	m_node = m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_button = (CButton*)m_node->m_object;

		m_button->CheckMouseOver();

		if (m_button->m_isMouseOver)
		{
			if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
			{
				return m_button->m_network;
			}
			else
			{
				return nullptr;
			}
		}

		m_node = m_node->m_next;
	}

	m_node = m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_textInput = (CTextInput*)m_node->m_object;

		m_textInput->CheckMouseOver();

		m_node = m_node->m_next;
	}

	if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
	{
		m_mouseDevice->ResetDrag();
	}

	if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count > 10)
	{
		CPanel::Drag();

		m_mouseDevice->UpdateDrag();
	}

	return nullptr;
}

/*
*/
void CPanel::AddButton(CButton* button)
{
	m_buttons->Add(button, button->m_name->m_text);
}

/*
*/
void CPanel::AddImage(CImage* image)
{
	m_images->Add(image, "");
}

/*
*/
void CPanel::AddTextInput(CTextInput* textInput)
{
	m_textInputs->Add(textInput, "");
}

/*
*/
CNetwork* CPanel::CheckMouseOver()
{
	if (m_background->m_box->CheckPointInBox(&m_cursor->m_position))
	{
		return CPanel::Active();
	}
	else
	{
		CPanel::Inactive();

		m_node = m_buttons->m_list;

		while ((m_node) && (m_node->m_object))
		{
			m_button = (CButton*)m_node->m_object;

			m_button->Inactive();

			m_node = m_node->m_next;
		}

		m_node = m_textInputs->m_list;

		while ((m_node) && (m_node->m_object))
		{
			m_textInput = (CTextInput*)m_node->m_object;

			m_textInput->m_isActive = false;
			m_textInput->m_image->m_highlight = 0.75f;

			m_node = m_node->m_next;
		}
	}

	return nullptr;
}

/*
*/
void CPanel::DisplayText()
{
	if (m_isMouseOver)
	{
		m_font->Draw(m_title->m_text, CVec2f(m_background->m_position.m_p.x + 100.0f, m_background->m_position.m_p.y + 10.0f), m_background->m_size, m_graphicsAdapter->m_rgbColor->SunYellow);
	}
	else
	{
		m_font->Draw(m_title->m_text, CVec2f(m_background->m_position.m_p.x + 100.0f, m_background->m_position.m_p.y + 10.0f), m_background->m_size, m_graphicsAdapter->m_rgbColor->ZombieGreen);
	}

	m_node = m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_button = (CButton*)m_node->m_object;

		m_button->DisplayText();

		m_node = m_node->m_next;
	}

	m_node = m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_textInput = (CTextInput*)m_node->m_object;

		m_textInput->DisplayText();

		m_node = m_node->m_next;
	}
}

/*
*/
void CPanel::Drag()
{
	m_background->Move(m_mouseDevice->m_hud.m_lX, m_mouseDevice->m_hud.m_lY);

	m_node = m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_button = (CButton*)m_node->m_object;

		m_button->Move(m_mouseDevice->m_hud.m_lX, m_mouseDevice->m_hud.m_lY);

		m_node = m_node->m_next;
	}

	m_node = m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_textInput = (CTextInput*)m_node->m_object;

		m_textInput->Move(m_mouseDevice->m_hud.m_lX, m_mouseDevice->m_hud.m_lY);

		m_node = m_node->m_next;
	}
}

/*
*/
void CPanel::Inactive()
{
	m_isMouseOver = false;

	m_background->m_highlight = 0.75f;
}

/*
*/
void CPanel::Record()
{
	m_background->Record();

	m_node = m_images->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_image = (CImage*)m_node->m_object;

		m_image->Record();

		m_node = m_node->m_next;
	}

	m_node = m_buttons->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_button = (CButton*)m_node->m_object;

		m_button->Record();

		m_node = m_node->m_next;
	}

	m_node = m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_textInput = (CTextInput*)m_node->m_object;

		m_textInput->Record();

		m_node = m_node->m_next;
	}
}

/*
*/
void CPanel::UserInput(char wParam)
{
	m_node = m_textInputs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_textInput = (CTextInput*)m_node->m_object;

		if (m_textInput->m_isActive)
		{
			if (wParam == VK_BACK)
			{
				m_textInput->Backspace();
			}
			else if (wParam == VK_RETURN)
			{
				m_textInput->Inactive();
			}
			else if ((wParam >= 32) && (wParam <= 126))
			{
				m_textInput->AddChar((char)wParam);
			}

			break;
		}

		m_node = m_node->m_next;
	}
}