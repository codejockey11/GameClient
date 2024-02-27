#pragma once

#include "framework.h"

#include "CColor.h"
#include "CErrorLog.h"
#include "CString.h"
#include "CVideoDevice.h"
#include "CVertex.h"

class CFont
{
public:

	CString* m_face;
	
	ComPtr<IDWriteTextFormat> m_textFormat;
	
	float m_height;

	CFont();
	CFont(CErrorLog* errorLog, CVideoDevice* device, CColor* color, const char* face, float pitch, DWRITE_FONT_WEIGHT weight);
	~CFont();

	void Draw(const wchar_t* text, CVertex2 position, CVertex2 size, CColor* color);

	SIZE TextDimensions(const char* text);

private:

	CColor* m_color;
	CErrorLog* m_errorLog;
	CVideoDevice* m_videoDevice;
	
	ComPtr<ID2D1SolidColorBrush> m_textBrush;
	
	D2D1_RECT_F m_textRect;
};