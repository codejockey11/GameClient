#pragma once

#include "framework.h"

#include "CErrorLog.h"
#include "CString.h"
#include "CWavFileHeader.h"

class CWavLoader
{
public:

	BYTE* m_data;
	
	CString* m_filename;
	CWavFileHeader m_header;
	
	WAVEFORMATEX m_wfx;

	CWavLoader();
	CWavLoader(CErrorLog* errorLog, const char* filename);
	~CWavLoader();

private:

	CErrorLog* m_errorLog;
};