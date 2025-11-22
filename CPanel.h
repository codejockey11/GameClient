#pragma once

#include "framework.h"

#include "../GameCommon/CList.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CButton.h"
#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CShaderBinaryManager.h"
#include "CTextInput.h"

class CPanel
{
public:

	bool m_isMouseOver;

	CButton* m_button;
	CErrorLog* m_errorLog;
	CFont* m_font;
	CGraphicsAdapter* m_graphicsAdapter;
	CImage* m_background;
	CImage* m_cursor;
	CImage* m_image;
	CList* m_buttons;
	CList* m_images;
	CList* m_textInputs;
	CListNode* m_node;
	CMouseDevice* m_mouseDevice;
	CString* m_title;
	CTextInput* m_textInput;
	CVec2f m_position;
	CVec2f m_size;

	CPanel();
	CPanel(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, int32_t bundle, CImage* cursor,
		CTexture* texture, CFont* font, const char* title, CVec2f position, CVec2f size);
	~CPanel();

	CNetwork* Active();
	void AddButton(CButton* button);
	void AddImage(CImage* image);
	void AddTextInput(CTextInput* textInput);
	CNetwork* CheckMouseOver();
	void DisplayText();
	void Drag();
	void Inactive();
	void Record();
	void UserInput(char wParam);
};