#include "CSoundManager.h"

/*
*/
CSoundManager::CSoundManager()
{
	memset(this, 0x00, sizeof(CSoundManager));
}

/*
*/
CSoundManager::CSoundManager(CSoundDevice* soundDevice, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CSoundManager));

	m_soundDevice = soundDevice;

	m_errorLog = errorLog;

	m_sounds = new CLinkList<CSound>();
}

/*
*/
CSoundManager::~CSoundManager()
{
	delete m_sounds;
}

/*
*/
CSound* CSoundManager::Create(const char* filename, BYTE loop)
{
	CSound* sound = CSoundManager::Get(filename);

	if (sound)
	{
		return sound;
	}

	sound = new CSound(m_errorLog, m_soundDevice, filename, loop);

	if (sound->m_initialized)
	{
		m_sounds->Add(sound, filename);

		return sound;
	}

	m_errorLog->WriteError(true, "CSoundManager::MakeSound:Error:%s\n", filename);

	delete sound;

	return nullptr;
}

/*
*/
void CSoundManager::Delete(const char* name)
{
	CLinkListNode<CSound>* lln = m_sounds->Search(name);

	if (lln)
	{
		m_sounds->Delete(lln);
	}
}

/*
*/
CSound* CSoundManager::Get(const char* filename)
{
	CLinkListNode<CSound>* lln = m_sounds->Search(filename);

	if (lln)
	{
		return lln->m_object;
	}

	return nullptr;
}