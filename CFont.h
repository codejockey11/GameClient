#pragma once

#include "framework.h"

#include "../GameCommon/CColor.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CGraphicsAdapter.h"

class CFont
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID2D1SolidColorBrush> m_textBrush;
	ComPtr<IDWriteTextFormat> m_textFormat;

	CString* m_face;

	D2D1_RECT_F m_textRect;

	HDC m_hdc;

	HFONT m_hFont;

	HRESULT m_hr;

	SIZE m_dimensionSize;

	size_t m_size;

	int32_t m_height;

	WCHAR m_localName[LOCALE_NAME_MAX_LENGTH];

	wchar_t* m_uniText;

	CFont();
	CFont(CErrorLog* errorLog, CGraphicsAdapter* device, const char* face, float pitch, DWRITE_FONT_WEIGHT weight);
	~CFont();

	void Draw(const char* text, CVec2f position, CVec2f size, CColor* color);
	SIZE TextDimensions(const char* text);
};