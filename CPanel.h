#pragma once

#include "framework.h"

#include "../GameCommon/CList.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CButton.h"
#include "CConstantBuffer.h"
#include "CFont.h"
#include "CGlyphText.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CPipelineState.h"
#include "CTextInput.h"

class CPanel
{
public:

	bool m_isMouseOver;

	CButton* m_button;
	CErrorLog* m_errorLog;
	CFont* m_font;
	CGlyphText* m_glyphText;
	CGraphicsAdapter* m_graphicsAdapter;
	CImage* m_background;
	CImage* m_cursor;
	CImage* m_image;
	CList* m_buttons;
	CList* m_images;
	CList* m_glyphTexts;
	CList* m_textInputs;
	CListNode* m_node;
	CMouseDevice* m_mouseDevice;
	CString* m_title;
	CTextInput* m_textInput;
	CVec2f m_position;
	CVec2f m_size;

	CPanel();
	CPanel(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, int32_t listGroup,
		CImage* cursor, CTexture* texture, CFont* font, const char* title, CVec2f position, CVec2f size);
	~CPanel();

	CNetwork* Active();
	void AddButton(CButton* button, const char* name);
	void AddImage(CImage* image, const char* name);
	void AddText(CGlyphText* text, const char* name);
	void AddTextInput(CTextInput* textInput, const char* name);
	CNetwork* CheckMouseOver();
	void DisplayText();
	void Drag();
	void Inactive();
	void Record();
	void UserInput(char wParam);
};