#include "CFontManager.h"

/*
*/
CFontManager::CFontManager()
{
	memset(this, 0x00, sizeof(CFontManager));
}

/*
*/
CFontManager::CFontManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CFontManager));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

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
	m_font = CFontManager::Get(name);

	if (m_font)
	{
		return m_font;
	}

	memset(m_name, 0x00, CFontManager::E_MAX_NAME);
	
	sscanf_s(name, "%s %i", m_name, CFontManager::E_MAX_NAME, &m_pitch);

	m_font = new CFont(m_errorLog, m_graphicsAdapter, m_name, (float)m_pitch, weight);

	if (m_font->m_textFormat)
	{
		m_fonts->Add(m_font, name);

		return m_font;
	}

	m_errorLog->WriteError(true, "CFontManager::Create:Error:%s\n", name);

	delete m_font;

	return nullptr;
}

/*
*/
void CFontManager::Delete(const char* name)
{
	m_fontNode = m_fonts->Search(name);

	if (m_fontNode)
	{
		m_fonts->Delete(m_fontNode);
	}
}

/*
*/
CFont* CFontManager::Get(const char* name)
{
	m_fontNode = m_fonts->Search(name);

	if (m_fontNode)
	{
		return (CFont*)m_fontNode->m_object;
	}

	return nullptr;
}