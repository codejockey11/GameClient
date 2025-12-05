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
	CListNode* m_wavLoaderNode;
	CLocal* m_local;
	CString* m_filename;
	CWavLoader* m_defaultWavLoader;
	CWavLoader* m_wavLoader;

	CSoundManager();
	CSoundManager(CErrorLog* errorLog, CLocal* local, const char* defaultSound);
	~CSoundManager();

	CWavLoader* Create(const char* name);
	void Delete(const char* name);
	CWavLoader* Get(const char* name);
};