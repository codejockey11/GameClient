#pragma once

#include "framework.h"

#include "../GameCommon/CColor.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CNetwork.h"
#include "../GameCommon/CString.h"

#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CShader.h"
#include "CTexture.h"
#include "CShaderBinaryManager.h"

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
	
	CVec2f m_bottomRight;
	CVec2f m_position;
	CVec2f m_size;
	CVec2f m_topLeft;
	
	bool m_isVisible;
	
	int m_cursorLocation;
	int m_entryNbr;
	int m_lines;
	int m_scroll;

	CChatBox();
	CChatBox(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CTexture* texture, CFont* font, float width, float height);
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

	CFont* m_font;
	CErrorLog* m_errorLog;
	CTexture* m_texture;
	CGraphicsAdapter* m_graphicsAdapter;
	
	int m_start;
};