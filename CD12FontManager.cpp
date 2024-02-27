#include "CD12FontManager.h"

/*
*/
CD12FontManager::CD12FontManager()
{
	memset(this, 0x00, sizeof(CD12FontManager));
}

/*
*/
CD12FontManager::CD12FontManager(CVideoDevice* videoDevice, CErrorLog* errorLog, CColor* color)
{
	memset(this, 0x00, sizeof(CD12FontManager));

	m_videoDevice = videoDevice;

	m_errorLog = errorLog;

	m_color = color;

	m_fonts = new CLinkList<CD12Font>();
}

/*
*/
CD12FontManager::~CD12FontManager()
{
	delete m_fonts;
}

/*
*/
CD12Font* CD12FontManager::Create(const char* name, DWRITE_FONT_WEIGHT weight)
{
	CD12Font* font = CD12FontManager::Get(name);

	if (font)
	{
		return font;
	}

	char fname[64] = {};
	int fpitch = 0;
	
	sscanf_s(name, "%s %i", fname, 64, &fpitch);

	font = new CD12Font(m_errorLog, m_videoDevice, m_color, fname, (float)fpitch, weight);

	if (font->m_textFormat)
	{
		m_fonts->Add(font, name);

		return font;
	}

	m_errorLog->WriteError(true, "CD12FontManager::Create:Error:%s\n", name);

	delete font;

	return nullptr;
}

/*
*/
void CD12FontManager::Delete(const char* name)
{
	CLinkListNode<CD12Font>* lln = m_fonts->Search(name);

	if (lln)
	{
		m_fonts->Delete(lln);
	}
}

/*
*/
CD12Font* CD12FontManager::Get(const char* name)
{
	CLinkListNode<CD12Font>* lln = m_fonts->Search(name);

	if (lln)
	{
		return (CD12Font*)lln->m_object;
	}

	return nullptr;
}