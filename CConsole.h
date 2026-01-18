#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CString.h"

#include "CConstantBuffer.h"
#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CPipelineState.h"
#include "CTexture.h"

class CConsole
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

	int32_t m_scroll;

	int32_t m_cursorLocation;
	int32_t m_entryNbr;
	int32_t m_lines;
	int32_t m_start;

	CConsole();
	CConsole(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, int32_t listGroup, CTexture* texture,
		CFont* font, int32_t width, int32_t height);
	~CConsole();

	void AddChar(char c);
	void AddEventMessage(const char* message);
	void Backspace();
	void Record();
	void DisplayText();
	void Paging(char direction);
	void Reset();
	void ToggleVisibility();
};