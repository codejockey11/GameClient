#include "CPanel.h"

/*
*/
CPanel::CPanel()
{
	memset(this, 0x00, sizeof(CPanel));
}

/*
*/
CPanel::CPanel(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CImage* cursor, CTexture* texture,
	CFont* font, const char* title, CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CPanel));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_cursor = cursor;

	m_font = font;

	m_background = new CImage(m_graphicsAdapter, m_errorLog, shaderBinaryManager, texture, position, size);

	m_title = new CString(title);

	m_buttons = new CLinkList<CButton>();

	m_images = new CLinkList<CImage>();

	m_textInputs = new CLinkList<CTextInput>();
}

/*
*/
CPanel::~CPanel()
{
	delete m_textInputs;
	delete m_images;
	delete m_buttons;
	delete m_title;
	delete m_background;
}

/*
*/
CNetwork* CPanel::Active()
{
	m_isMouseOver = true;

	m_buttonNode = m_buttons->m_list;

	while (m_buttonNode->m_object)
	{
		m_buttonNode->m_object->CheckMouseOver();

		if (m_buttonNode->m_object->m_isMouseOver)
		{
			if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
			{
				return m_buttonNode->m_object->m_network;
			}
		}

		m_buttonNode = m_buttonNode->m_next;
	}

	m_textInputNode = m_textInputs->m_list;

	while (m_textInputNode->m_object)
	{
		m_textInputNode->m_object->CheckMouseOver();

		m_textInputNode = m_textInputNode->m_next;
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

		m_buttonNode = m_buttons->m_list;

		while (m_buttonNode->m_object)
		{
			m_buttonNode->m_object->Inactive();

			m_buttonNode = m_buttonNode->m_next;
		}

		m_textInputNode = m_textInputs->m_list;

		while (m_textInputNode->m_object)
		{
			m_textInputNode->m_object->m_isActive = false;
			m_textInputNode->m_object->m_image->m_highlight = 0.75f;

			m_textInputNode = m_textInputNode->m_next;
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
		m_font->Draw(m_title->m_text, CVec2f(m_background->m_position.m_p.x, m_background->m_position.m_p.y), m_background->m_size, m_graphicsAdapter->m_color->SunYellow);
	}
	else
	{
		m_font->Draw(m_title->m_text, CVec2f(m_background->m_position.m_p.x, m_background->m_position.m_p.y), m_background->m_size, m_graphicsAdapter->m_color->ZombieGreen);
	}

	m_buttonNode = m_buttons->m_list;

	while (m_buttonNode->m_object)
	{
		m_buttonNode->m_object->DisplayText();

		m_buttonNode = m_buttonNode->m_next;
	}

	m_textInputNode = m_textInputs->m_list;

	while (m_textInputNode->m_object)
	{
		m_textInputNode->m_object->DisplayText();

		m_textInputNode = m_textInputNode->m_next;
	}
}

/*
*/
void CPanel::Drag()
{
	m_background->Move(m_mouseDevice->m_hud.m_lX, m_mouseDevice->m_hud.m_lY);

	m_buttonNode = m_buttons->m_list;

	while (m_buttonNode->m_object)
	{
		m_buttonNode->m_object->m_image->Move(m_mouseDevice->m_hud.m_lX, m_mouseDevice->m_hud.m_lY);

		m_buttonNode = m_buttonNode->m_next;
	}

	m_textInputNode = m_textInputs->m_list;

	while (m_textInputNode->m_object)
	{
		m_textInputNode->m_object->m_image->Move(m_mouseDevice->m_hud.m_lX, m_mouseDevice->m_hud.m_lY);

		m_textInputNode = m_textInputNode->m_next;
	}
}

/*
*/
void CPanel::Inactive()
{
	m_isMouseOver = false;
}

/*
*/
void CPanel::Record()
{
	m_background->Record();

	m_graphicsAdapter->BundleCommandList(4, m_background->m_commandList);

	m_imageNode = m_images->m_list;

	while (m_imageNode->m_object)
	{
		m_imageNode->m_object->Record();

		m_graphicsAdapter->BundleCommandList(4, m_imageNode->m_object->m_commandList);

		m_imageNode = m_imageNode->m_next;
	}

	m_buttonNode = m_buttons->m_list;

	while (m_buttonNode->m_object)
	{
		m_buttonNode->m_object->Record();

		m_graphicsAdapter->BundleCommandList(4, m_buttonNode->m_object->m_image->m_commandList);

		m_buttonNode = m_buttonNode->m_next;
	}

	m_textInputNode = m_textInputs->m_list;

	while (m_textInputNode->m_object)
	{
		m_textInputNode->m_object->Record();

		m_graphicsAdapter->BundleCommandList(4, m_textInputNode->m_object->m_image->m_commandList);

		m_textInputNode = m_textInputNode->m_next;
	}
}

/*
*/
void CPanel::UserInput(char wParam)
{
	m_textInputNode = m_textInputs->m_list;

	while (m_textInputNode->m_object)
	{
		if (m_textInputNode->m_object->m_isActive)
		{
			if (wParam == VK_BACK)
			{
				m_textInputNode->m_object->Backspace();
			}
			else if (wParam == VK_RETURN)
			{
				m_textInputNode->m_object->Inactive();
			}
			else if ((wParam >= 32) && (wParam <= 126))
			{
				m_textInputNode->m_object->AddChar((char)wParam);
			}

			break;
		}

		m_textInputNode = m_textInputNode->m_next;
	}
}