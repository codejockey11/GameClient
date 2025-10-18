#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"

#include "CWavefront.h"

class CWavefrontManager
{
public:

	CErrorLog* m_errorLog;
	CLinkList<CWavefront>* m_models;
	CLocal* m_local;

	CWavefrontManager();
	CWavefrontManager(CErrorLog* errorLog, CLocal* local);
	~CWavefrontManager();

	CWavefront* Create(char* filename);
	void Delete(char* filename);
	CWavefront* Get(char* filename);
};