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
	m_wavLoaderNode = m_wavLoaders->m_list;

	while ((m_wavLoaderNode) && (m_wavLoaderNode->m_object))
	{
		m_wavLoader = (CWavLoader*)m_wavLoaderNode->m_object;

		SAFE_DELETE(m_wavLoader);

		m_wavLoaderNode = m_wavLoaders->Delete(m_wavLoaderNode);
	}

	SAFE_DELETE(m_wavLoaders);
}

/*
*/
CWavLoader* CSoundManager::Create(const char* name)
{
	m_errorLog->WriteError(true, "CSoundManager::Create:%s\n", name);

	m_wavLoader = CSoundManager::Get(name);

	if (m_wavLoader)
	{
		m_errorLog->WriteError(true, "CSoundManager::Create:Returned\n");

		return m_wavLoader;
	}

	m_filename = new CString(m_local->m_installPath->m_text);

	m_filename->Append("main/");
	m_filename->Append(name);

	m_wavLoader = new CWavLoader(m_errorLog, name, m_filename->m_text);

	SAFE_DELETE(m_filename);

	if (m_wavLoader->m_isInitialized)
	{
		m_wavLoaders->Add(m_wavLoader, name);

		m_errorLog->WriteError(true, "CSoundManager::Create:Completed\n");

		return m_wavLoader;
	}

	SAFE_DELETE(m_wavLoader);

	m_errorLog->WriteError(true, "CSoundManager::Create:Failed\n");

	return m_defaultWavLoader;
}

/*
*/
void CSoundManager::Delete(const char* name)
{
	m_wavLoaderNode = m_wavLoaders->Search(name);

	if (m_wavLoaderNode)
	{
		m_wavLoader = (CWavLoader*)m_wavLoaderNode->m_object;

		SAFE_DELETE(m_wavLoader);

		m_wavLoaders->Delete(m_wavLoaderNode);
	}
}

/*
*/
CWavLoader* CSoundManager::Get(const char* name)
{
	m_wavLoaderNode = m_wavLoaders->Search(name);

	if (m_wavLoaderNode)
	{
		return (CWavLoader*)m_wavLoaderNode->m_object;
	}

	return nullptr;
}