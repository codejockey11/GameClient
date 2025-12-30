#include "CSoundManager.h"

/*
*/
CSoundManager::CSoundManager()
{
	memset(this, 0x00, sizeof(CSoundManager));
}

/*
*/
CSoundManager::CSoundManager(CErrorLog* errorLog, CLocal* local, const char* defaultSound)
{
	memset(this, 0x00, sizeof(CSoundManager));

	m_errorLog = errorLog;

	m_local = local;

	m_wavLoaders = new CList();

	m_defaultWavLoader = CSoundManager::Create(defaultSound);
}

/*
*/
CSoundManager::~CSoundManager()
{
	CListNode* node = m_wavLoaders->m_list;

	while ((node) && (node->m_object))
	{
		CWavLoader* wavLoader = (CWavLoader*)node->m_object;

		SAFE_DELETE(wavLoader);

		node = m_wavLoaders->Delete(node);
	}

	SAFE_DELETE(m_wavLoaders);
}

/*
*/
CWavLoader* CSoundManager::Create(const char* name)
{
	CWavLoader* wavLoader = CSoundManager::Get(name);

	if (wavLoader != m_defaultWavLoader)
	{
		return wavLoader;
	}

	m_errorLog->WriteError(true, "CSoundManager::Create:%s\n", name);

	CString* filename = new CString(m_local->m_installPath->m_text);

	filename->Append("main/");
	filename->Append(name);

	wavLoader = new CWavLoader(m_errorLog, name, filename->m_text);

	SAFE_DELETE(filename);

	if (wavLoader->m_isInitialized)
	{
		m_wavLoaders->Add(wavLoader, name);

		return wavLoader;
	}

	SAFE_DELETE(wavLoader);

	m_errorLog->WriteError(true, "CSoundManager::Create:Failed\n");

	return m_defaultWavLoader;
}

/*
*/
void CSoundManager::Delete(const char* name)
{
	CListNode* node = m_wavLoaders->Search(name);

	if ((node) && (node->m_object))
	{
		CWavLoader* wavLoader = (CWavLoader*)node->m_object;

		SAFE_DELETE(wavLoader);

		m_wavLoaders->Delete(node);
	}
}

/*
*/
CWavLoader* CSoundManager::Get(const char* name)
{
	CListNode* node = m_wavLoaders->Search(name);

	if ((node) && (node->m_object))
	{
		return (CWavLoader*)node->m_object;
	}

	return m_defaultWavLoader;
}