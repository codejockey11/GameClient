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

	m_wavLoaders = new CLinkList<CWavLoader>();
}

/*
*/
CSoundManager::~CSoundManager()
{
	delete m_wavLoaders;
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

	delete m_wavLoader;

	return nullptr;
}

/*
*/
void CSoundManager::Delete(const char* name)
{
	m_wavLoaderNode = m_wavLoaders->Search(name);

	if (m_wavLoaderNode)
	{
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
		return m_wavLoaderNode->m_object;
	}

	return nullptr;
}