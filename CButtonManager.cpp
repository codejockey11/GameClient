#include "CButtonManager.h"

/*
*/
CButtonManager::CButtonManager()
{
	memset(this, 0x00, sizeof(CButtonManager));
}

/*
*/
CButtonManager::CButtonManager(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager,
	CShaderBinaryManager* shaderBinaryManager)
{
	memset(this, 0x00, sizeof(CButtonManager));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_fontManager = fontManager;

	m_soundDevice = soundDevice;

	m_shaderBinaryManager = shaderBinaryManager;

	m_buttons = new CList();
}

/*
*/
CButtonManager::~CButtonManager()
{
	m_buttonNode = m_buttons->m_list;

	while ((m_buttonNode) && (m_buttonNode->m_object))
	{
		m_button = (CButton*)m_buttonNode->m_object;

		SAFE_DELETE(m_button);

		m_buttonNode = m_buttons->Delete(m_buttonNode);
	}

	SAFE_DELETE(m_buttons);
}

/*
*/
CButton* CButtonManager::Create(int bundle, const char* name, CFont* font, CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image, CVec2f position, CVec2f size,
	BYTE audience, BYTE type)
{
	m_button = CButtonManager::Get(name);

	if (m_button)
	{
		return m_button;
	}

	m_button = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_shaderBinaryManager, bundle, name, font, soundOn, soundOff, soundClicked, image, position, size,
		audience, type);

	if (m_button->m_isInitialized)
	{
		m_buttons->Add(m_button, name);

		return m_button;
	}

	m_errorLog->WriteError(true, "CButtonManager::Create:Error:%s\n", name);

	SAFE_DELETE(m_button);

	return nullptr;
}

/*
*/
void CButtonManager::Delete(const char* name)
{
	m_buttonNode = m_buttons->Search(name);

	if (m_buttonNode)
	{
		m_button = (CButton*)m_buttonNode->m_object;

		SAFE_DELETE(m_button);

		m_buttons->Delete(m_buttonNode);
	}
}

/*
*/
CButton* CButtonManager::Get(const char* name)
{
	m_buttonNode = m_buttons->Search(name);

	if (m_buttonNode)
	{
		return (CButton*)m_buttonNode->m_object;
	}

	return nullptr;
}