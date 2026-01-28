#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CString.h"

#include "CWavFileHeader.h"

class CWavLoader
{
public:

	bool m_isInitialized;

	BYTE* m_data;

	CErrorLog* m_errorLog;

	char m_garbage[32];

	CString* m_name;
	CWavFileHeader m_header;

	errno_t m_err;

	FILE* m_fWav;

	WAVEFORMATEX m_wfx;

	CWavLoader();
	CWavLoader(CErrorLog* errorLog, const char* name, const char* filename);
	~CWavLoader();
};