#pragma once

#include "framework.h"

#include "CComError.h"
#include "CDXGIError.h"
#include "CWinsockError.h"
#include "CXAudio2Error.h"

class CErrorLog
{
public:

	enum
	{
		E_TIME_LENGTH = 24,
		E_LINE_LENGTH = 1024
	};

	CErrorLog();
	CErrorLog(const char* filename);
	~CErrorLog();

	void WriteBytes(bool time, const char* bytes);
	void WriteComErrorMessage(bool time, const char* format, HRESULT hr);
	void WriteDXGIErrorMessage(bool time, const char* format, int error);
	void WriteError(bool time, const char* format, ...);
	void WriteWinsockErrorMessage(bool time, const char* format);
	void WriteXAudio2ErrorMessage(bool time, const char* format, int error);

private:

	CComError* m_comError;
	CDXGIError* m_dxgiError;
	CWinsockError* m_wsError;
	CXAudio2Error* m_xaudio2Error;
	
	FILE* m_file;
	
	char m_text[CErrorLog::E_LINE_LENGTH];
};