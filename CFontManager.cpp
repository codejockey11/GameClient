#include "CFontManager.h"

/*
*/
CFontManager::CFontManager()
{
	memset(this, 0x00, sizeof(CFontManager));
}

/*
*/
CFontManager::CFontManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, const char* defaultFont, DWRITE_FONT_WEIGHT weight)
{
	memset(this, 0x00, sizeof(CFontManager));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_fonts = new CList();

	m_defaultFont = CFontManager::Create(defaultFont, weight);
}

/*
*/
CFontManager::~CFontManager()
{
	CListNode* node = m_fonts->m_list;

	while ((node) && (node->m_object))
	{
		CFont* font = (CFont*)node->m_object;

		SAFE_DELETE(font);

		node = m_fonts->Delete(node);
	}

	SAFE_DELETE(m_fonts);
}

/*
*/
CFont* CFontManager::Create(const char* name, DWRITE_FONT_WEIGHT weight)
{
	CFont* font = CFontManager::Get(name);

	if (font != m_defaultFont)
	{
		return font;
	}

	m_errorLog->WriteError(true, "CFontManager::Create:%s\n", name);

	memset(m_name, 0x00, CFontManager::E_MAX_NAME);
	
	sscanf_s(name, "%s %i", m_name, CFontManager::E_MAX_NAME, &m_pitch);

	font = new CFont(m_errorLog, m_graphicsAdapter, m_name, (float)m_pitch, weight);

	if (font->m_textFormat)
	{
		m_fonts->Add(font, name);

		return font;
	}

	m_errorLog->WriteError(true, "CFontManager::Create:Error:%s\n", name);

	SAFE_DELETE(font);

	return m_defaultFont;
}

/*
*/
void CFontManager::Delete(const char* name)
{
	CListNode* node = m_fonts->Search(name);

	if ((node) && (node->m_object))
	{
		CFont* font = (CFont*)node->m_object;

		SAFE_DELETE(font);

		m_fonts->Delete(node);
	}
}

/*
*/
CFont* CFontManager::Get(const char* name)
{
	CListNode* node = m_fonts->Search(name);

	if ((node) && (node->m_object))
	{
		return (CFont*)node->m_object;
	}

	return m_defaultFont;
}