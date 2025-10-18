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

	CString* m_face;
	
	ComPtr<IDWriteTextFormat> m_textFormat;
	
	float m_height;

	CFont();
	CFont(CErrorLog* errorLog, CGraphicsAdapter* device, const char* face, float pitch, DWRITE_FONT_WEIGHT weight);
	~CFont();

	void Draw(const char* text, CVec2f position, CVec2f size, CColor* color);

	SIZE TextDimensions(const char* text);

private:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	
	ComPtr<ID2D1SolidColorBrush> m_textBrush;
	
	D2D1_RECT_F m_textRect;

	HRESULT m_hr;
};