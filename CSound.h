#pragma once

#include "framework.h"

#include "CErrorLog.h"
#include "CSoundDevice.h"
#include "CString.h"
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

	CString* m_filename;

	BYTE m_initialized;

	CSound();
	CSound(CErrorLog* errorLog, CSoundDevice* soundDevice, const char* filename, bool loop);
	~CSound();

	bool IsPlaying();
	void SetEffectChain();
	void SetEffectParameters();
	void Shutdown();
	void StartSound();
	void StopSound();
	void ToggleEffect(BYTE soundEffect);

private:

	BYTE m_isOn[CSound::E_EFFECT_COUNT];
	
	CErrorLog* m_errorLog;
	CSoundDevice* m_soundDevice;
	CWavLoader* m_wavLoader;
	
	FXECHO_PARAMETERS m_xapoEcho;
	
	FXREVERB_PARAMETERS m_xapoReverb;
	
	IUnknown* m_xapoEffect[CSound::E_EFFECT_COUNT];
	
	IXAudio2SourceVoice* m_sourceVoice[CSound::E_SOURCE_VOICE_COUNT];
	
	XAUDIO2FX_REVERB_PARAMETERS	m_reverbParameters;
	
	XAUDIO2_BUFFER m_buffer;
	XAUDIO2_EFFECT_CHAIN m_effectChain;
	XAUDIO2_EFFECT_DESCRIPTOR m_effectDesc[CSound::E_EFFECT_COUNT];
	XAUDIO2_VOICE_STATE m_voiceState;
};