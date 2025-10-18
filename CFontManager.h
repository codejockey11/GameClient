#pragma once

#include "framework.h"

#include "../GameCommon/CColor.h"
#include "../GameCommon/CLinkList.h"

#include "CFont.h"
#include "CGraphicsAdapter.h"

class CFontManager
{
public:

	CLinkList<CFont>* m_fonts;

	CFontManager();
	CFontManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CFontManager();

	CFont* Create(const char* name, DWRITE_FONT_WEIGHT weight);
	void Delete(const char* name);
	CFont* Get(const char* name);

private:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
};