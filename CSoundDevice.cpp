#include "CSoundDevice.h"

/* Initialize XAudio2
hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

if (hr != S_OK)
{
	errorLog->WriteError("CSoundDevice::CSoundDevice::CoInitializeEx:%s\n", errorLog->GetComErrorMessage(hr));

	return;
}

CoUninitialize();
*/

/*
*/
CSoundDevice::CSoundDevice()
{
	memset(this, 0x00, sizeof(CSoundDevice));
}

/*
*/
CSoundDevice::CSoundDevice(CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CSoundDevice));

	m_errorLog = errorLog;


	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

	if (hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::CoCreateInstance:", hr);

		return;
	}

	hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &m_pCollection);

	if (hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::EnumAudioEndpoints:", hr);

		return;
	}

	hr = m_pCollection->GetCount(&m_count);

	if (hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::GetCount:", hr);

		return;
	}

	for (UINT i = 0; i < m_count; i++)
	{
		hr = m_pCollection->Item(i, &m_pEndpoint);

		hr = m_pEndpoint->GetId(&m_pwszID);

		hr = m_pEndpoint->OpenPropertyStore(STGM_READ, &m_pProps);

		PropVariantInit(&m_varName);

		hr = m_pProps->GetValue(PKEY_Device_FriendlyName, &m_varName);

		m_errorLog->WriteError(true, "CSoundDevice::DeviceInformation::%d:%S\n", i, m_varName.pwszVal);
		m_errorLog->WriteError(true, "CSoundDevice::DeviceInformation::%d:%S\n", i, m_pwszID);

		CoTaskMemFree(m_pwszID);
		m_pwszID = nullptr;
		PropVariantClear(&m_varName);

		m_pProps->Release();
		m_pEndpoint->Release();
	}

	m_pEnumerator->Release();
	m_pCollection->Release();

#ifdef _DEBUG
	hr = XAudio2Create(&m_xAudio2, XAUDIO2_DEBUG_ENGINE, XAUDIO2_DEFAULT_PROCESSOR);
#else
	hr = XAudio2Create(&m_xAudio2, 0x0000, XAUDIO2_DEFAULT_PROCESSOR);
#endif

	if (hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::XAudio2Create:", hr);

		return;
	}

	hr = m_xAudio2->CreateMasteringVoice(&m_masteringVoice);

	if (hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::CreateMasteringVoice:", hr);

		return;
	}

	m_masteringVoice->GetChannelMask(&m_channelMask);

	X3DAudioInitialize(m_channelMask, X3DAUDIO_SPEED_OF_SOUND, m_3DAudio);
}

/*
*/
CSoundDevice::~CSoundDevice()
{
	if (m_xAudio2)
	{
		m_xAudio2->Release();
	}
}