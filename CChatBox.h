#pragma once

#include "framework.h"

#include "CColor.h"
#include "CFont.h"
#include "CErrorLog.h"
#include "CImage.h"
#include "CLinkList.h"
#include "CNetwork.h"
#include "CShader.h"
#include "CString.h"
#include "CTexture.h"
#include "CVideoDevice.h"

class CChatBox
{
public:

	enum
	{
		E_ENTRY_LENGTH = 256,
		E_PAGE_BACKWARD = 0,
		E_PAGE_FORWARD = 1
	};

	CImage* m_background;
	CLinkList<CString>* m_entries;
	CString* m_message;
	
	CVertex2 m_bottomRight;
	CVertex2 m_position;
	CVertex2 m_size;
	CVertex2 m_topLeft;
	
	bool m_isVisible;
	
	int m_cursorLocation;
	int m_entryNbr;
	int m_lines;
	int m_scroll;

	CChatBox();
	CChatBox(CVideoDevice* videoDevice, CErrorLog* errorLog, CTexture* texture, CFont* font, CColor* color, CShaderManager* shaderManager, float width, float height);
	~CChatBox();

	void AddChar(char c);
	void AddEventMessage(const char* message);
	void Backspace();
	void DisplayConsole();
	void DisplayText();
	void Paging(char direction);
	void Reset();
	void ToggleVisibility();

private:

	CColor* m_color;
	CFont* m_font;
	CErrorLog* m_errorLog;
	CShaderManager* m_shaderManager;
	CTexture* m_texture;
	CVideoDevice* m_videoDevice;
	
	char m_text[CChatBox::E_ENTRY_LENGTH];
	
	int m_start;
};