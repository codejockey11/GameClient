#include "CPanel.h"

/*
*/
CPanel::CPanel()
{
	memset(this, 0x00, sizeof(CPanel));
}

/*
*/
CPanel::CPanel(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CImage* cursor, CImage* background, CFont* font, const char* title)
{
	memset(this, 0x00, sizeof(CPanel));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_cursor = cursor;

	m_background = background;

	m_font = font;

	m_title = new CString(title);

	m_buttons = new CLinkList<CButton>();

	m_images = new CLinkList<CImage>();

	m_textInputs = new CLinkList<CTextInput>();

	CVec3f v1(m_background->m_position.m_p.x, m_background->m_position.m_p.y, 0.0f);
	CVec3f v2(m_background->m_position.m_p.x + m_background->m_size.m_p.x, m_background->m_position.m_p.y + m_background->m_size.m_p.y, 0.0f);

	m_box = new CBoundBox(&v1, &v2);
}

/*
*/
CPanel::~CPanel()
{
	delete m_title;
	delete m_box;
	delete m_background;
	delete m_buttons;
	delete m_images;
	delete m_textInputs;
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
CNetwork* CPanel::Active()
{
	m_isMouseOver = true;

	CLinkListNode<CButton>* button = m_buttons->m_list;

	while (button->m_object)
	{
		button->m_object->CheckMouseOver();

		if (button->m_object->m_isMouseOver)
		{
			if (m_mouseDevice->m_keyMap[CMouseDevice::E_LMB].m_count == 1)
			{
				return button->m_object->m_network;
			}
		}

		button = button->m_next;
	}

	CLinkListNode<CTextInput>* textInput = m_textInputs->m_list;

	while (textInput->m_object)
	{
		textInput->m_object->CheckMouseOver();

		textInput = textInput->m_next;
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
CNetwork* CPanel::CheckMouseOver()
{
	if (m_box->CheckPointInBox(&m_cursor->m_position))
	{
		return CPanel::Active();
	}
	else
	{
		CPanel::Inactive();

		CLinkListNode<CButton>* button = m_buttons->m_list;

		while (button->m_object)
		{
			button->m_object->Inactive();

			button = button->m_next;
		}

		CLinkListNode<CTextInput>* textInput = m_textInputs->m_list;

		while (textInput->m_object)
		{
			textInput->m_object->m_isActive = false;
			textInput->m_object->m_image->m_highlight = 0.75f;

			textInput = textInput->m_next;
		}
	}

	return nullptr;
}

/*
*/
void CPanel::Drag()
{
	m_background->m_position.m_p.x += m_mouseDevice->m_hud.m_lX;
	m_background->m_position.m_p.y += m_mouseDevice->m_hud.m_lY;

	m_background->UpdatePosition();

	CVec3f v1(m_background->m_position.m_p.x, m_background->m_position.m_p.y, 0.0f);
	CVec3f v2(m_background->m_position.m_p.x + m_background->m_size.m_p.x, m_background->m_position.m_p.y + m_background->m_size.m_p.y, 0.0f);

	m_box->Update(&v1, &v2);

	CLinkListNode<CButton>* button = m_buttons->m_list;

	while (button->m_object)
	{
		button->m_object->m_image->m_position.m_p.x += m_mouseDevice->m_hud.m_lX;
		button->m_object->m_image->m_position.m_p.y += m_mouseDevice->m_hud.m_lY;

		button->m_object->m_image->UpdatePosition();

		CVec3f vertex1(button->m_object->m_image->m_position.m_p.x, button->m_object->m_image->m_position.m_p.y, 0.0f);
		CVec3f vertex2(button->m_object->m_image->m_position.m_p.x + button->m_object->m_image->m_size.m_p.x, button->m_object->m_image->m_position.m_p.y + button->m_object->m_image->m_size.m_p.y, 0.0f);

		button->m_object->m_box->Update(&vertex1, &vertex2);

		button = button->m_next;
	}

	CLinkListNode<CTextInput>* textInput = m_textInputs->m_list;

	while (textInput->m_object)
	{
		textInput->m_object->m_position.m_p.x += m_mouseDevice->m_hud.m_lX;
		textInput->m_object->m_position.m_p.y += m_mouseDevice->m_hud.m_lY;

		textInput->m_object->m_image->m_position.m_p.x += m_mouseDevice->m_hud.m_lX;
		textInput->m_object->m_image->m_position.m_p.y += m_mouseDevice->m_hud.m_lY;

		textInput->m_object->m_image->UpdatePosition();

		CVec3f vertex1(textInput->m_object->m_image->m_position.m_p.x, textInput->m_object->m_image->m_position.m_p.y, 0.0f);
		CVec3f vertex2(textInput->m_object->m_image->m_position.m_p.x + textInput->m_object->m_image->m_size.m_p.x, textInput->m_object->m_image->m_position.m_p.y + textInput->m_object->m_image->m_size.m_p.y, 0.0f);

		textInput->m_object->m_box->Update(&vertex1, &vertex2);

		textInput = textInput->m_next;
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

	CLinkListNode<CImage>* image = m_images->m_list;

	while (image->m_object)
	{
		image->m_object->Record();

		m_graphicsAdapter->BundleCommandList(4, image->m_object->m_commandList);

		image = image->m_next;
	}

	CLinkListNode<CButton>* button = m_buttons->m_list;

	while (button->m_object)
	{
		button->m_object->Record();

		m_graphicsAdapter->BundleCommandList(4, button->m_object->m_image->m_commandList);

		button = button->m_next;
	}

	CLinkListNode<CTextInput>* textInput = m_textInputs->m_list;

	while (textInput->m_object)
	{
		textInput->m_object->Record();

		m_graphicsAdapter->BundleCommandList(4, textInput->m_object->m_image->m_commandList);

		textInput = textInput->m_next;
	}
}

/*
*/
void CPanel::DisplayText()
{
	SIZE s = m_font->TextDimensions(m_title->m_text);

	if (m_isMouseOver)
	{
		m_font->Draw(m_title->m_text, CVec2f(m_background->m_position.m_p.x, m_background->m_position.m_p.y), m_background->m_size, m_graphicsAdapter->m_color->SunYellow);
	}
	else
	{
		m_font->Draw(m_title->m_text, CVec2f(m_background->m_position.m_p.x, m_background->m_position.m_p.y), m_background->m_size, m_graphicsAdapter->m_color->ZombieGreen);
	}

	CLinkListNode<CButton>* button = m_buttons->m_list;

	while (button->m_object)
	{
		button->m_object->DisplayText();

		button = button->m_next;
	}

	CLinkListNode<CTextInput>* textInput = m_textInputs->m_list;

	while (textInput->m_object)
	{
		textInput->m_object->DisplayText();

		textInput = textInput->m_next;
	}
}

/*
*/
void CPanel::UserInput(char wParam)
{
	CLinkListNode<CTextInput>* textInput = m_textInputs->m_list;

	while (textInput->m_object)
	{
		if (textInput->m_object->m_isActive)
		{
			if (wParam == VK_BACK)
			{
				textInput->m_object->Backspace();
			}
			else if (wParam == VK_RETURN)
			{
				textInput->m_object->Inactive();
			}
			else if ((wParam >= 32) && (wParam <= 126))
			{
				textInput->m_object->AddChar((char)wParam);
			}

			break;
		}

		textInput = textInput->m_next;
	}
}