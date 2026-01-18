#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"

#include "CWavefront.h"

class CWavefrontManager
{
public:

	CErrorLog* m_errorLog;
	CList* m_wavefronts;
	CLocal* m_local;
	CWavefront* m_defaultWavefront;

	CWavefrontManager();
	CWavefrontManager(CErrorLog* errorLog, CLocal* local, const char* deafaultWavefront);
	~CWavefrontManager();

	CWavefront* Create(const char* name);
	void Delete(const char* name);
	CWavefront* Get(const char* name);
};