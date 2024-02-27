#pragma once

#include "framework.h"

#include "CColor.h"
#include "CFont.h"
#include "CLinkList.h"
#include "CVideoDevice.h"

class CFontManager
{
public:

	CLinkList<CFont>* m_fonts;

	CFontManager();
	CFontManager(CVideoDevice* videoDevice, CErrorLog* errorLog, CColor* color);
	~CFontManager();

	CFont* Create(const char* name, DWRITE_FONT_WEIGHT weight);
	void Delete(const char* name);
	CFont* Get(const char* name);

private:

	CColor* m_color;
	CErrorLog* m_errorLog;
	CVideoDevice* m_videoDevice;
};