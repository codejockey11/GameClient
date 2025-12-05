#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

class CSoundDevice
{
public:
	
	CErrorLog* m_errorLog;

	DWORD m_channelMask;

	HRESULT m_hr;

	IMMDevice* m_pEndpoint;
	IMMDeviceCollection* m_pCollection;
	IMMDeviceEnumerator* m_pEnumerator;

	IPropertyStore* m_pProps;

	IXAudio2* m_xAudio2;
	IXAudio2MasteringVoice* m_masteringVoice;

	LPWSTR m_pwszID;

	PROPVARIANT m_varName;

	int32_t m_count;

	X3DAUDIO_HANDLE m_3DAudio;

	CSoundDevice();
	CSoundDevice(CErrorLog* errorLog);
	~CSoundDevice();
};