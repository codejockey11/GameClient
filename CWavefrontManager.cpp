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

	m_models = new CLinkList<CWavefront>();

	m_local = local;
}

/*
*/
CWavefrontManager::~CWavefrontManager()
{
	delete m_models;
}

/*
*/
CWavefront* CWavefrontManager::Create(char* filename)
{
	CWavefront* model = CWavefrontManager::Get(filename);

	if (model)
	{
		return model;
	}

	model = new CWavefront(m_local, filename);

	if (model->m_isInitialized)
	{
		m_models->Add(model, filename);

		return model;
	}

	delete model;

	m_errorLog->WriteError(true, "CWavefrontManager::MakeTexture:%s\n", filename);

	return nullptr;
}

/*
*/
void CWavefrontManager::Delete(char* filename)
{
	CLinkListNode<CWavefront>* lln = m_models->Search(filename);

	if (lln)
	{
		m_models->Delete(lln);
	}
}

/*
*/
CWavefront* CWavefrontManager::Get(char* filename)
{
	CLinkListNode<CWavefront>* lln = m_models->Search(filename);

	if (lln)
	{
		return lln->m_object;
	}

	return nullptr;
}