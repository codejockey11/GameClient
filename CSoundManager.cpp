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

	m_sounds = new CLinkList<CWavLoader>();
}

/*
*/
CSoundManager::~CSoundManager()
{
	delete m_sounds;
}

/*
*/
CWavLoader* CSoundManager::Create(const char* name)
{
	CWavLoader* sound = CSoundManager::Get(name);

	if (sound)
	{
		return sound;
	}

	sound = new CWavLoader(m_errorLog, m_local, name);

	if (sound->m_initialized)
	{
		m_sounds->Add(sound, name);

		return sound;
	}

	m_errorLog->WriteError(true, "CSoundManager::Create:%s\n", name);

	delete sound;

	return nullptr;
}

/*
*/
void CSoundManager::Delete(const char* name)
{
	CLinkListNode<CWavLoader>* lln = m_sounds->Search(name);

	if (lln)
	{
		m_sounds->Delete(lln);
	}
}

/*
*/
CWavLoader* CSoundManager::Get(const char* name)
{
	CLinkListNode<CWavLoader>* lln = m_sounds->Search(name);

	if (lln)
	{
		return lln->m_object;
	}

	return nullptr;
}