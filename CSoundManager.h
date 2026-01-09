#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"

#include "CWavLoader.h"

class CSoundManager
{
public:

	CErrorLog* m_errorLog;
	CList* m_wavLoaders;
	CLocal* m_local;
	CWavLoader* m_defaultWavLoader;

	CSoundManager();
	CSoundManager(CErrorLog* errorLog, CLocal* local, const char* defaultSound);
	~CSoundManager();

	CWavLoader* Create(const char* name);
	void Delete(const char* name);
	CWavLoader* Get(const char* name);
};