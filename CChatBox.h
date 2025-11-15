#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CString.h"

#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CShaderBinaryManager.h"
#include "CTexture.h"

class CChatBox
{
public:

	enum
	{
		E_ENTRY_LENGTH = 256,
		E_PAGE_BACKWARD = 0,
		E_PAGE_FORWARD = 1
	};

	bool m_isVisible;

	CErrorLog* m_errorLog;
	CFont* m_font;
	CGraphicsAdapter* m_graphicsAdapter;
	CImage* m_background;
	CList* m_entries;
	CListNode* m_node;
	CString* m_message;
	CString* m_listMessage;
	CTexture* m_texture;
	CVec2f m_bottomRight;
	CVec2f m_position;
	CVec2f m_size;
	CVec2f m_textPosition;
	CVec2f m_topLeft;

	int m_cursorLocation;
	int m_entryNbr;
	int m_lines;
	int m_scroll;
	int m_start;

	CChatBox();
	CChatBox(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, int bundle, CTexture* texture, CFont* font,
		float width, float height);
	~CChatBox();

	void AddChar(char c);
	void AddEventMessage(const char* message);
	void Backspace();
	void DisplayConsole();
	void DisplayText();
	void Paging(char direction);
	void Reset();
	void ToggleVisibility();
};