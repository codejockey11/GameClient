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

	GetSystemDefaultLocaleName(m_localName, LOCALE_NAME_MAX_LENGTH);

#ifdef D3D11on12
	m_uniText = new wchar_t[strlen(m_face->m_text) + 1]();

	mbstowcs_s(&m_size, m_uniText, strlen(m_face->m_text) + 1, m_face->m_text, strlen(m_face->m_text));
	
	m_hr = m_graphicsAdapter->m_d3d11On12->m_dWriteFactory->CreateTextFormat(m_uniText,
		nullptr,
		weight,
		DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
		pitch,
		m_localName,
		&m_textFormat);
		
	SAFE_DELETE_ARRAY(m_uniText);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CFont::CFont::CreateTextFormat:", m_hr);
		
		m_errorLog->WriteError(true, "face:%s\n", face);

		return;
	}

	m_height = (int32_t)m_textFormat->GetFontSize();

	m_hr = m_graphicsAdapter->m_d3d11On12->m_d2dDeviceContext->CreateSolidColorBrush(
		D2D1::ColorF(
			m_graphicsAdapter->m_rgbColor->White->m_RGBA[0],
			m_graphicsAdapter->m_rgbColor->White->m_RGBA[1],
			m_graphicsAdapter->m_rgbColor->White->m_RGBA[2],
			m_graphicsAdapter->m_rgbColor->White->m_RGBA[3]),
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

	SAFE_DELETE(m_face);
}

/*
*/
void CFont::Draw(const char* text, CVec2f position, CVec2f size, CColor* color)
{
	m_textRect = D2D1::RectF(position.m_p.x, position.m_p.y, position.m_p.x + size.m_p.x, position.m_p.y + size.m_p.y);

	m_textBrush->SetColor(D2D1::ColorF(color->m_RGBA[0], color->m_RGBA[1], color->m_RGBA[2], color->m_RGBA[3]));

#ifdef D3D11on12
	if (m_textFormat && m_textBrush)
	{
		m_uniText = new wchar_t[strlen(text) + 1]();

		mbstowcs_s(&m_size, m_uniText, strlen(text) + 1, text, strlen(text));

		m_graphicsAdapter->m_d3d11On12->m_d2dDeviceContext->DrawTextW(m_uniText,
			(int32_t)wcslen(m_uniText),
			m_textFormat.Get(),
			&m_textRect,
			m_textBrush.Get());

		SAFE_DELETE_ARRAY(m_uniText);
	}
#endif
}

/*
*/
SIZE CFont::TextDimensions(const char* text)
{
	m_hdc = GetDC(m_graphicsAdapter->m_hWnd);

	m_hFont = CreateFontA(-MulDiv((int)m_height, GetDeviceCaps(m_hdc, LOGPIXELSY), (int)GetDpiForWindow(m_graphicsAdapter->m_hWnd)), 0,
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

	SelectObject(m_hdc, m_hFont);

	GetTextExtentPoint32A(m_hdc, text, (int)strlen(text), &m_dimensionSize);

	DeleteObject(m_hFont);
	ReleaseDC(m_graphicsAdapter->m_hWnd, m_hdc);

	return m_dimensionSize;
}