#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"

#include "CWavLoader.h"

class CSoundManager
{
public:

	CErrorLog* m_errorLog;
	CLinkList<CWavLoader>* m_sounds;
	CLocal* m_local;

	CSoundManager();
	CSoundManager(CErrorLog* errorLog, CLocal* local);
	~CSoundManager();

	CWavLoader* Create(const char* name);
	void Delete(const char* name);
	CWavLoader* Get(const char* name);
};