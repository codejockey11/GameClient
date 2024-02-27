#include "CFontManager.h"

/*
*/
CFontManager::CFontManager()
{
	memset(this, 0x00, sizeof(CFontManager));
}

/*
*/
CFontManager::CFontManager(CVideoDevice* videoDevice, CErrorLog* errorLog, CColor* color)
{
	memset(this, 0x00, sizeof(CFontManager));

	m_videoDevice = videoDevice;

	m_errorLog = errorLog;

	m_color = color;

	m_fonts = new CLinkList<CFont>();
}

/*
*/
CFontManager::~CFontManager()
{
	delete m_fonts;
}

/*
*/
CFont* CFontManager::Create(const char* name, DWRITE_FONT_WEIGHT weight)
{
	CFont* font = CFontManager::Get(name);

	if (font)
	{
		return font;
	}

	char fname[64] = {};
	int fpitch = 0;
	
	sscanf_s(name, "%s %i", fname, 64, &fpitch);

	font = new CFont(m_errorLog, m_videoDevice, m_color, fname, (float)fpitch, weight);

	if (font->m_textFormat)
	{
		m_fonts->Add(font, name);

		return font;
	}

	m_errorLog->WriteError(true, "CFontManager::Create:Error:%s\n", name);

	delete font;

	return nullptr;
}

/*
*/
void CFontManager::Delete(const char* name)
{
	CLinkListNode<CFont>* lln = m_fonts->Search(name);

	if (lln)
	{
		m_fonts->Delete(lln);
	}
}

/*
*/
CFont* CFontManager::Get(const char* name)
{
	CLinkListNode<CFont>* lln = m_fonts->Search(name);

	if (lln)
	{
		return (CFont*)lln->m_object;
	}

	return nullptr;
}