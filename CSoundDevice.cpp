#include "CSoundDevice.h"

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

	m_hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::CoCreateInstance:", m_hr);

		return;
	}

	m_hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &m_pCollection);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::EnumAudioEndpoints:", m_hr);

		return;
	}

	m_hr = m_pCollection->GetCount((uint32_t*)&m_count);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::GetCount:", m_hr);

		return;
	}

	for (int32_t i = 0; i < m_count; i++)
	{
		m_hr = m_pCollection->Item(i, &m_pEndpoint);

		m_hr = m_pEndpoint->GetId(&m_pwszID);

		m_hr = m_pEndpoint->OpenPropertyStore(STGM_READ, &m_pProps);

		PropVariantInit(&m_varName);

		m_hr = m_pProps->GetValue(PKEY_Device_FriendlyName, &m_varName);

		m_errorLog->WriteError(true, "CSoundDevice::DeviceInformation::%d:%S\n", i, m_varName.pwszVal);
		
		m_errorLog->WriteError(true, "CSoundDevice::DeviceInformation::%d:%S\n", i, m_pwszID);

		CoTaskMemFree(m_pwszID);
		
		PropVariantClear(&m_varName);

		m_pProps->Release();
		
		m_pEndpoint->Release();
	}

	m_pEnumerator->Release();
	
	m_pCollection->Release();

#ifdef _DEBUG
	m_hr = XAudio2Create(&m_xAudio2, XAUDIO2_DEBUG_ENGINE, XAUDIO2_DEFAULT_PROCESSOR);
#else
	m_hr = XAudio2Create(&m_xAudio2, 0x0000, XAUDIO2_DEFAULT_PROCESSOR);
#endif

	if (m_hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::XAudio2Create:", m_hr);

		return;
	}

	m_hr = m_xAudio2->CreateMasteringVoice(&m_masteringVoice);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteComErrorMessage(true, "CSoundDevice::CSoundDevice::CreateMasteringVoice:", m_hr);

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
		if (m_masteringVoice)
		{
			m_masteringVoice->DestroyVoice();
		}

		m_xAudio2->StopEngine();

		m_xAudio2->Release();
	}
}