#include "CWavefrontManager.h"

/*
*/
CWavefrontManager::CWavefrontManager()
{
	memset(this, 0x00, sizeof(CWavefrontManager));
}

/*
*/
CWavefrontManager::CWavefrontManager(CErrorLog* errorLog, CLocal* local)
{
	memset(this, 0x00, sizeof(CWavefrontManager));

	m_errorLog = errorLog;

	m_wavefronts = new CLinkList<CWavefront>();

	m_local = local;
}

/*
*/
CWavefrontManager::~CWavefrontManager()
{
	delete m_wavefronts;
}

/*
*/
CWavefront* CWavefrontManager::Create(char* filename)
{
	m_wavefront = CWavefrontManager::Get(filename);

	if (m_wavefront)
	{
		return m_wavefront;
	}

	m_wavefront = new CWavefront(m_local, filename);

	if (m_wavefront->m_isInitialized)
	{
		m_wavefronts->Add(m_wavefront, filename);

		return m_wavefront;
	}

	delete m_wavefront;

	m_errorLog->WriteError(true, "CWavefrontManager::MakeTexture:%s\n", filename);

	return nullptr;
}

/*
*/
void CWavefrontManager::Delete(char* filename)
{
	m_wavefrontNode = m_wavefronts->Search(filename);

	if (m_wavefrontNode)
	{
		m_wavefronts->Delete(m_wavefrontNode);
	}
}

/*
*/
CWavefront* CWavefrontManager::Get(char* filename)
{
	m_wavefrontNode = m_wavefronts->Search(filename);

	if (m_wavefrontNode)
	{
		return m_wavefrontNode->m_object;
	}

	return nullptr;
}