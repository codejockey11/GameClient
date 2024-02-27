#include "CButtonManager.h"

/*
*/
CButtonManager::CButtonManager()
{
	memset(this, 0x00, sizeof(CButtonManager));
}

/*
*/
CButtonManager::CButtonManager(CVideoDevice* videoDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager, CShaderManager* shaderManager,
	CSoundManager* soundManager, CColor* color)
{
	memset(this, 0x00, sizeof(CButtonManager));

	m_videoDevice = videoDevice;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_fontManager = fontManager;

	m_shaderManager = shaderManager;

	m_soundManager = soundManager;

	m_color = color;

	m_buttons = new CLinkList<CButton>();
}

/*
*/
CButtonManager::~CButtonManager()
{
	delete m_buttons;
}

/*
*/
CButton* CButtonManager::Create(const char* name, CTexture* image, const char* fontName, CVertex2 size, CVertex2 position, BYTE audience, BYTE type)
{
	CButton* button = CButtonManager::Get(name);

	if (button)
	{
		return button;
	}

	button = new CButton(m_videoDevice, m_mouseDevice, m_errorLog, m_fontManager, m_shaderManager, m_soundManager, m_color,
		name, image, fontName, size, position, audience, type);

	if (button->m_isInitialized)
	{
		m_buttons->Add(button, name);

		return button;
	}

	m_errorLog->WriteError(true, "CButtonManager::Create:Error:%s\n", name);

	delete button;

	return nullptr;
}

/*
*/
void CButtonManager::Delete(const char* name)
{
	CLinkListNode<CButton>* lln = m_buttons->Search(name);

	if (lln)
	{
		m_buttons->Delete(lln);
	}
}

/*
*/
CButton* CButtonManager::Get(const char* name)
{
	CLinkListNode<CButton>* lln = m_buttons->Search(name);

	if (lln)
	{
		return (CButton*)lln->m_object;
	}

	return nullptr;
}