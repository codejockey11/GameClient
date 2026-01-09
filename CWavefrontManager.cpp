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
	CListNode* node = m_wavefronts->m_list;

	while ((node) && (node->m_object))
	{
		CWavefront* wavefront = (CWavefront*)node->m_object;

		SAFE_DELETE(wavefront);

		node = m_wavefronts->Delete(node);
	}

	SAFE_DELETE(m_wavefronts);
}

/*
*/
CWavefront* CWavefrontManager::Create(const char* name)
{
	CWavefront* wavefront = CWavefrontManager::Get(name);

	if (wavefront != m_defaultWavefront)
	{
		return wavefront;
	}

	m_errorLog->WriteError(true, "CWavefrontManager::Create:%s\n", name);

	CString* filename = new CString(m_local->m_installPath->m_text);

	filename->Append("main/");
	filename->Append(name);

	wavefront = new CWavefront(name, filename->m_text);

	SAFE_DELETE(filename);

	filename = new CString(m_local->m_installPath->m_text);

	filename->Append("main/");
	filename->Append(wavefront->m_mtllib);

	wavefront->InitializeMaterialScript(filename->m_text);

	SAFE_DELETE(filename);

	wavefront->Load();

	if (wavefront->m_isInitialized)
	{
		m_wavefronts->Add(wavefront, name);

		return wavefront;
	}

	SAFE_DELETE(wavefront);

	m_errorLog->WriteError(true, "CWavefrontManager::Create:Failed\n");

	return m_defaultWavefront;
}

/*
*/
void CWavefrontManager::Delete(const char* name)
{
	CListNode* node = m_wavefronts->Search(name);

	if ((node) && (node->m_object))
	{
		CWavefront* wavefront = (CWavefront*)node->m_object;

		SAFE_DELETE(wavefront);

		m_wavefronts->Delete(node);
	}
}

/*
*/
CWavefront* CWavefrontManager::Get(const char* name)
{
	CListNode* node = m_wavefronts->Search(name);

	if ((node) && (node->m_object))
	{
		return (CWavefront*)node->m_object;
	}

	return m_defaultWavefront;
}