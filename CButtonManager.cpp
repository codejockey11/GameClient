#include "CButtonManager.h"

/*
*/
CButtonManager::CButtonManager()
{
	memset(this, 0x00, sizeof(CButtonManager));
}

/*
*/
CButtonManager::CButtonManager(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager, CShaderBinaryManager* shaderBinaryManager)
{
	memset(this, 0x00, sizeof(CButtonManager));

	m_graphicsAdapter = graphicsAdapter;

	m_mouseDevice = mouseDevice;

	m_errorLog = errorLog;

	m_fontManager = fontManager;

	m_soundDevice = soundDevice;

	m_shaderBinaryManager = shaderBinaryManager;

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
CButton* CButtonManager::Create(const char* name, CFont* font,
	CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image, CVec2f position, CVec2f size, BYTE audience, BYTE type)
{
	CButton* button = CButtonManager::Get(name);

	if (button)
	{
		return button;
	}

	button = new CButton(m_graphicsAdapter, m_soundDevice, m_mouseDevice, m_errorLog, m_shaderBinaryManager, name, font, soundOn, soundOff, soundClicked, image, position, size, audience, type);

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