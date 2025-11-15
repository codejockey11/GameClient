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

	m_wavefronts = new CList();

	m_local = local;
}

/*
*/
CWavefrontManager::~CWavefrontManager()
{
	m_wavefrontNode = m_wavefronts->m_list;

	while ((m_wavefrontNode) && (m_wavefrontNode->m_object))
	{
		m_wavefront = (CWavefront*)m_wavefrontNode->m_object;

		SAFE_DELETE(m_wavefront);

		m_wavefrontNode = m_wavefronts->Delete(m_wavefrontNode);
	}

	SAFE_DELETE(m_wavefronts);
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

	SAFE_DELETE(m_wavefront);

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
		m_wavefront = (CWavefront*)m_wavefrontNode->m_object;

		SAFE_DELETE(m_wavefront);

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
		return (CWavefront*)m_wavefrontNode->m_object;
	}

	return nullptr;
}