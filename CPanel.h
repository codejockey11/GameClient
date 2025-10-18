#pragma once

#include "framework.h"

#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CBoundBox.h"
#include "CButton.h"
#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CTextInput.h"

class CPanel
{
public:

	CBoundBox* m_box;
	CErrorLog* m_errorLog;
	CFont* m_font;
	CGraphicsAdapter* m_graphicsAdapter;
	CImage* m_background;
	CImage* m_cursor;
	CMouseDevice* m_mouseDevice;
	CString* m_title;
	CVec2f m_position;
	CVec2f m_size;

	bool m_isMouseOver;

	CLinkList<CButton>* m_buttons;
	CLinkList<CImage>* m_images;
	CLinkList<CTextInput>* m_textInputs;

	CPanel();
	CPanel(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CImage* cursor, CImage* background, CFont* font, const char* title);
	~CPanel();

	void AddButton(CButton* button);
	void AddImage(CImage* image);
	void AddTextInput(CTextInput* textInput);

	CNetwork* Active();
	CNetwork* CheckMouseOver();
	void Drag();
	void Inactive();

	void DisplayText();
	void Record();

	void UserInput(char wParam);
};