#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CSoundDevice.h"
#include "CWavLoader.h"

class CSound
{
public:

	enum
	{
		E_SE_ECHO = 0,
		E_SE_REVERB,
		E_SE_REVERBXAPO,
		E_EFFECT_COUNT = 3,
		E_SOURCE_VOICE_COUNT = 16
	};

	bool m_isInitialized;
	bool m_isOn[CSound::E_EFFECT_COUNT];

	CErrorLog* m_errorLog;
	CSoundDevice* m_soundDevice;
	CWavLoader* m_wavLoader;

	FXECHO_PARAMETERS m_xapoEcho;
	FXREVERB_PARAMETERS m_xapoReverb;

	HRESULT m_hr;

	IUnknown* m_xapoEffect[CSound::E_EFFECT_COUNT];

	IXAudio2SourceVoice* m_sourceVoice[CSound::E_SOURCE_VOICE_COUNT];

	XAUDIO2_BUFFER m_buffer;
	XAUDIO2_EFFECT_CHAIN m_effectChain;
	XAUDIO2_EFFECT_DESCRIPTOR m_effectDesc[CSound::E_EFFECT_COUNT];
	XAUDIO2_VOICE_STATE m_voiceState;

	XAUDIO2FX_REVERB_PARAMETERS	m_reverbParameters;

	CSound();
	CSound(CErrorLog* errorLog, CSoundDevice* soundDevice, CWavLoader* wavLoader, bool loop);
	~CSound();

	void SetEffectChain();
	void SetEffectParameters();
	void Shutdown();
	void StartSound();
	void StopSound();
	void ToggleEffect(BYTE soundEffect);
};