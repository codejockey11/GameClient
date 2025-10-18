#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CString.h"

#include "CWavFileHeader.h"

class CWavLoader
{
public:
	
	CErrorLog* m_errorLog;
	CLocal* m_local;
	CString* m_filename;
	CString* m_name;
	CWavFileHeader m_header;

	bool m_initialized;

	BYTE* m_data;

	char m_garbage[32];

	errno_t m_err;

	FILE* m_fWav;

	WAVEFORMATEX m_wfx;

	CWavLoader();
	CWavLoader(CErrorLog* errorLog, CLocal* local, const char* name);
	~CWavLoader();
};