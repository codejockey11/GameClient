#include "CSoundManager.h"

/*
*/
CSoundManager::CSoundManager()
{
	memset(this, 0x00, sizeof(CSoundManager));
}

/*
*/
CSoundManager::CSoundManager(CErrorLog* errorLog, CLocal* local)
{
	memset(this, 0x00, sizeof(CSoundManager));

	m_errorLog = errorLog;

	m_local = local;

	m_wavLoaders = new CList();
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
	m_wavLoader = CSoundManager::Get(name);

	if (m_wavLoader)
	{
		return m_wavLoader;
	}

	m_wavLoader = new CWavLoader(m_errorLog, m_local, name);

	if (m_wavLoader->m_isInitialized)
	{
		m_wavLoaders->Add(m_wavLoader, name);

		return m_wavLoader;
	}

	m_errorLog->WriteError(true, "CSoundManager::Create:%s\n", name);

	SAFE_DELETE(m_wavLoader);

	return nullptr;
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