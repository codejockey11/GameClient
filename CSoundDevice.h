#pragma once

#include "framework.h"

#include "CErrorLog.h"

class CSoundDevice
{
public:

	DWORD m_channelMask;
	
	IXAudio2* m_xAudio2;
	
	IXAudio2MasteringVoice* m_masteringVoice;
	
	X3DAUDIO_HANDLE m_3DAudio;

	CSoundDevice();
	CSoundDevice(CErrorLog* errorLog);
	~CSoundDevice();

private:

	CErrorLog* m_errorLog;
	
	IMMDevice* m_pEndpoint;
	IMMDeviceCollection* m_pCollection;
	IMMDeviceEnumerator* m_pEnumerator;
	
	IPropertyStore* m_pProps;
	
	LPWSTR m_pwszID;
	
	PROPVARIANT m_varName;
	
	UINT32 m_count;
};