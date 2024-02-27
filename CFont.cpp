#include "CFont.h"

/*
*/
CFont::CFont()
{
	memset(this, 0x00, sizeof(CFont));
}

/*
*/
CFont::CFont(CErrorLog* errorLog, CVideoDevice* videoDevice, CColor* color,
	const char* face, float pitch, DWRITE_FONT_WEIGHT weight)
{
	memset(this, 0x00, sizeof(CFont));

	m_errorLog = errorLog;

	m_videoDevice = videoDevice;

	m_color = color;

	m_face = new CString(face);


	WCHAR localName[LOCALE_NAME_MAX_LENGTH];

	GetSystemDefaultLocaleName(localName, LOCALE_NAME_MAX_LENGTH);

	HRESULT hr = m_videoDevice->m_d3d11On12->m_dWriteFactory->CreateTextFormat(m_face->GetWText(),
		nullptr,
		weight,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
		pitch,
		localName,
		&m_textFormat);

	if (hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CFont::CFont::CreateTextFormat:", hr);
		
		m_errorLog->WriteError(true, "face:%s\n", face);

		return;
	}

	m_height = m_textFormat->GetFontSize();

	hr = m_videoDevice->m_d3d11On12->m_d2dDeviceContext->CreateSolidColorBrush(
		D2D1::ColorF(m_color->White->m_RGBA[0], m_color->White->m_RGBA[1], m_color->White->m_RGBA[2], m_color->White->m_RGBA[3]),
		&m_textBrush);

	if (hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CFont::CFont::CreateSolidColorBrush:", hr);

		m_errorLog->WriteError(true, "face:%s\n", face);
	}
}

/*
*/
CFont::~CFont()
{
	m_textBrush.Reset();

	m_textFormat.Reset();

	delete m_face;
}

/*
*/
void CFont::Draw(const wchar_t* text, CVertex2 position, CVertex2 size, CColor* color)
{
	m_textRect = D2D1::RectF(position.p.x, position.p.y, position.p.x + size.p.x, position.p.y + size.p.y);

	m_textBrush->SetColor(D2D1::ColorF(color->m_RGBA[0], color->m_RGBA[1], color->m_RGBA[2], color->m_RGBA[3]));

	if (m_textFormat && m_textBrush)
	{
		m_videoDevice->m_d3d11On12->m_d2dDeviceContext->DrawText(text,
			(UINT32)wcslen(text),
			m_textFormat.Get(),
			&m_textRect,
			m_textBrush.Get());
	}
}

/*
*/
SIZE CFont::TextDimensions(const char* text)
{
	SIZE size = {};

	HDC hdc = GetDC(m_videoDevice->m_hWnd);

	// This matches the CFont 
	HFONT hFont = CreateFontA(
		-MulDiv((int)m_height, GetDeviceCaps(hdc, LOGPIXELSY), (int)GetDpiForWindow(m_videoDevice->m_hWnd)), //calculate the actual cHeight.
		0, 0, 0, // normal orientation
		m_textFormat->GetFontWeight(),   // normal weight--e.g., bold would be FW_BOLD
		false, false, false, // not italic, underlined or strike out
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, // select only outline (not bitmap) fonts
		CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, VARIABLE_PITCH, m_face->GetText());

	SelectObject(hdc, hFont);

	GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size);

	DeleteObject(hFont); //always delete the object after creating it.
	ReleaseDC(m_videoDevice->m_hWnd, hdc); //alway reelase dc after using.

	return size;
}