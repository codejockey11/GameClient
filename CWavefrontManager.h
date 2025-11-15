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
	CListNode* m_wavefrontNode;
	CLocal* m_local;
	CWavefront* m_wavefront;

	CWavefrontManager();
	CWavefrontManager(CErrorLog* errorLog, CLocal* local);
	~CWavefrontManager();

	CWavefront* Create(char* filename);
	void Delete(char* filename);
	CWavefront* Get(char* filename);
};