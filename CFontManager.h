#pragma once

#include "framework.h"

#include "../GameCommon/CList.h"

#include "CFont.h"
#include "CGraphicsAdapter.h"

class CFontManager
{
public:

	enum
	{
		E_MAX_NAME = 64
	};

	CErrorLog* m_errorLog;
	CFont* m_font;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_name[CFontManager::E_MAX_NAME];

	CList* m_fonts;
	CListNode* m_fontNode;

	int m_pitch;

	CFontManager();
	CFontManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CFontManager();

	CFont* Create(const char* name, DWRITE_FONT_WEIGHT weight);
	void Delete(const char* name);
	CFont* Get(const char* name);
};