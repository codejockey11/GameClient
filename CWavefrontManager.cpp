#include "CWavefrontManager.h"

/*
*/
CWavefrontManager::CWavefrontManager()
{
	memset(this, 0x00, sizeof(CWavefrontManager));
}

/*
*/
CWavefrontManager::CWavefrontManager(CErrorLog* errorLog, CLocal* local, const char* deafaultWavefront)
{
	memset(this, 0x00, sizeof(CWavefrontManager));

	m_errorLog = errorLog;

	m_wavefronts = new CList();

	m_local = local;

	m_defaultWavefront = CWavefrontManager::Create(deafaultWavefront);
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
CWavefront* CWavefrontManager::Create(const char* name)
{
	m_errorLog->WriteError(true, "CWavefrontManager::Create:%s\n", name);

	m_wavefront = CWavefrontManager::Get(name);

	if (m_wavefront)
	{
		m_errorLog->WriteError(true, "CWavefrontManager::Create:Returned\n");

		return m_wavefront;
	}

	m_filename = new CString(m_local->m_installPath->m_text);

	m_filename->Append("main/");
	m_filename->Append(name);

	m_wavefront = new CWavefront(name, m_filename->m_text);

	SAFE_DELETE(m_filename);

	m_filename = new CString(m_local->m_installPath->m_text);

	m_filename->Append("main/");
	m_filename->Append(m_wavefront->m_mtllib);

	m_wavefront->InitializeMaterialScript(m_filename->m_text);

	SAFE_DELETE(m_filename);

	m_wavefront->Load();

	if (m_wavefront->m_isInitialized)
	{
		m_wavefronts->Add(m_wavefront, name);

		m_errorLog->WriteError(true, "CWavefrontManager::Create:Completed\n");

		return m_wavefront;
	}

	SAFE_DELETE(m_wavefront);

	m_errorLog->WriteError(true, "CWavefrontManager::Create:Failed\n");

	return m_defaultWavefront;
}

/*
*/
void CWavefrontManager::Delete(const char* name)
{
	m_wavefrontNode = m_wavefronts->Search(name);

	if (m_wavefrontNode)
	{
		m_wavefront = (CWavefront*)m_wavefrontNode->m_object;

		SAFE_DELETE(m_wavefront);

		m_wavefronts->Delete(m_wavefrontNode);
	}
}

/*
*/
CWavefront* CWavefrontManager::Get(const char* name)
{
	m_wavefrontNode = m_wavefronts->Search(name);

	if (m_wavefrontNode)
	{
		return (CWavefront*)m_wavefrontNode->m_object;
	}

	return nullptr;
}