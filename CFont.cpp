#include "CFont.h"

/*
*/
CFont::CFont()
{
	memset(this, 0x00, sizeof(CFont));
}

/*
*/
CFont::CFont(CErrorLog* errorLog, CGraphicsAdapter* graphicsAdapter, const char* face, float pitch, DWRITE_FONT_WEIGHT weight)
{
	memset(this, 0x00, sizeof(CFont));

	m_errorLog = errorLog;

	m_graphicsAdapter = graphicsAdapter;

	m_face = new CString(face);


	WCHAR localName[LOCALE_NAME_MAX_LENGTH];

	GetSystemDefaultLocaleName(localName, LOCALE_NAME_MAX_LENGTH);
#ifdef DX11on12
	wchar_t* uniText = new wchar_t[strlen(m_face->m_text) + 1]();

	size_t size = 0;

	mbstowcs_s(&size, uniText, strlen(m_face->m_text) + 1, m_face->m_text, strlen(m_face->m_text));

	
	m_hr = m_graphicsAdapter->m_d3d11On12->m_dWriteFactory->CreateTextFormat(uniText,
		nullptr,
		weight,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
		pitch,
		localName,
		&m_textFormat);
		
	delete[] uniText;

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CFont::CFont::CreateTextFormat:", m_hr);
		
		m_errorLog->WriteError(true, "face:%s\n", face);

		return;
	}

	m_height = m_textFormat->GetFontSize();

	m_hr = m_graphicsAdapter->m_d3d11On12->m_d2dDeviceContext->CreateSolidColorBrush(
		D2D1::ColorF(
			m_graphicsAdapter->m_color->White->m_RGBA[0],
			m_graphicsAdapter->m_color->White->m_RGBA[1],
			m_graphicsAdapter->m_color->White->m_RGBA[2],
			m_graphicsAdapter->m_color->White->m_RGBA[3]),
		&m_textBrush);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CFont::CFont::CreateSolidColorBrush:", m_hr);

		m_errorLog->WriteError(true, "face:%s\n", face);
	}
#endif
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
void CFont::Draw(const char* text, CVec2f position, CVec2f size, CColor* color)
{
	m_textRect = D2D1::RectF(position.m_p.x, position.m_p.y, position.m_p.x + size.m_p.x, position.m_p.y + size.m_p.y);

	m_textBrush->SetColor(D2D1::ColorF(color->m_RGBA[0], color->m_RGBA[1], color->m_RGBA[2], color->m_RGBA[3]));
#ifdef DX11on12
	if (m_textFormat && m_textBrush)
	{
		wchar_t* uniText = new wchar_t[strlen(text) + 1]();

		size_t wsize = 0;

		mbstowcs_s(&wsize, uniText, strlen(text) + 1, text, strlen(text));

		m_graphicsAdapter->m_d3d11On12->m_d2dDeviceContext->DrawText(uniText,
			(UINT32)wcslen(uniText),
			m_textFormat.Get(),
			&m_textRect,
			m_textBrush.Get());

		delete[] uniText;
	}
#endif
}

/*
*/
SIZE CFont::TextDimensions(const char* text)
{
	SIZE size = {};

	HDC hdc = GetDC(m_graphicsAdapter->m_hWnd);

	HFONT hFont = CreateFontA(-MulDiv((int)m_height, GetDeviceCaps(hdc, LOGPIXELSY), (int)GetDpiForWindow(m_graphicsAdapter->m_hWnd)), 0,
		0,
		0,
		m_textFormat->GetFontWeight(),
		false,
		false,
		false,
		DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_NATURAL_QUALITY,
		VARIABLE_PITCH,
		m_face->m_text);

	SelectObject(hdc, hFont);

	GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size);

	DeleteObject(hFont);
	ReleaseDC(m_graphicsAdapter->m_hWnd, hdc);

	return size;
}