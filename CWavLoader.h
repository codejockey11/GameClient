#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"

#include "CWavFileHeader.h"

class CWavLoader
{
public:

	bool m_isInitialized;

	BYTE* m_data;

	CErrorLog* m_errorLog;

	char m_garbage[32];

	CLocal* m_local;
	CString* m_filename;
	CString* m_name;
	CWavFileHeader m_header;

	errno_t m_err;

	FILE* m_fWav;

	WAVEFORMATEX m_wfx;

	CWavLoader();
	CWavLoader(CErrorLog* errorLog, CLocal* local, const char* name);
	~CWavLoader();
};