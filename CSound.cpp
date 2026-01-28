#include "CSound.h"

/*
*/
CSound::CSound()
{
	memset(this, 0x00, sizeof(CSound));
}

/*
*/
CSound::CSound(CErrorLog* errorLog, CSoundDevice* soundDevice, CWavLoader* wavLoader, bool loop)
{
	memset(this, 0x00, sizeof(CSound));

	m_errorLog = errorLog;

	m_soundDevice = soundDevice;

	m_wavLoader = wavLoader;

	for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
	{
		m_hr = m_soundDevice->m_xAudio2->CreateSourceVoice(&m_sourceVoice[i], &m_wavLoader->m_wfx);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::CSound::CreateSourceVoice:", m_hr);

			return;
		}
	}

	m_buffer.AudioBytes = m_wavLoader->m_header.m_subchunk2Size;
	m_buffer.Flags = XAUDIO2_END_OF_STREAM;
	m_buffer.pAudioData = m_wavLoader->m_data;

	if (loop)
	{
		m_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	m_hr = CreateFX(__uuidof(FXEcho), &m_xapoEffect[CSound::E_SE_ECHO]);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::CSound::CreateFX::FXEcho:", m_hr);

		return;
	}

	m_hr = XAudio2CreateReverb(&m_xapoEffect[CSound::E_SE_REVERB]);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::CSound::XAudio2CreateReverb:", m_hr);

		return;
	}

	m_hr = CreateFX(__uuidof(FXReverb), &m_xapoEffect[CSound::E_SE_REVERBXAPO]);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::CSound::CreateFX::FXReverb:", m_hr);

		return;
	}

	m_xapoEcho.Delay = 500.0f;
	m_xapoEcho.Feedback = 0.50f;
	m_xapoEcho.WetDryMix = 0.50f;

	m_effectDesc[CSound::E_SE_ECHO].InitialState = false;
	m_effectDesc[CSound::E_SE_ECHO].OutputChannels = m_wavLoader->m_wfx.nChannels;
	m_effectDesc[CSound::E_SE_ECHO].pEffect = m_xapoEffect[CSound::E_SE_ECHO];

	m_reverbParameters.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;
	m_reverbParameters.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;
	m_reverbParameters.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
	m_reverbParameters.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	m_reverbParameters.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	m_reverbParameters.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	m_reverbParameters.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	m_reverbParameters.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
	m_reverbParameters.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
	m_reverbParameters.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
	m_reverbParameters.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
	m_reverbParameters.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
	m_reverbParameters.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
	m_reverbParameters.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;
	m_reverbParameters.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;
	m_reverbParameters.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;
	m_reverbParameters.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;
	m_reverbParameters.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN;
	m_reverbParameters.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME;
	m_reverbParameters.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY;
	m_reverbParameters.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE;
	m_reverbParameters.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX;

	m_effectDesc[CSound::E_SE_REVERB].InitialState = false;
	m_effectDesc[CSound::E_SE_REVERB].OutputChannels = m_wavLoader->m_wfx.nChannels;
	m_effectDesc[CSound::E_SE_REVERB].pEffect = m_xapoEffect[CSound::E_SE_REVERB];

	m_xapoReverb.Diffusion = FXREVERB_DEFAULT_DIFFUSION;
	m_xapoReverb.RoomSize = FXREVERB_DEFAULT_ROOMSIZE;

	m_effectDesc[CSound::E_SE_REVERBXAPO].InitialState = false;
	m_effectDesc[CSound::E_SE_REVERBXAPO].OutputChannels = m_wavLoader->m_wfx.nChannels;
	m_effectDesc[CSound::E_SE_REVERBXAPO].pEffect = m_xapoEffect[CSound::E_SE_REVERBXAPO];

	m_effectChain.EffectCount = CSound::E_EFFECT_COUNT;
	m_effectChain.pEffectDescriptors = m_effectDesc;

	CSound::SetEffectChain();
	CSound::SetEffectParameters();

	//CSound::ToggleEffect(CSound::E_SE_ECHO);
	//CSound::ToggleEffect(CSound::E_SE_REVERB);
	//CSound::ToggleEffect(CSound::E_SE_REVERBXAPO);

	m_isInitialized = true;
}

/*
*/
CSound::~CSound()
{
	CSound::StopSound();
	CSound::Shutdown();

	if (m_xapoEffect[CSound::E_SE_REVERB])
	{
		m_xapoEffect[CSound::E_SE_REVERB]->Release();
	}

	if (m_xapoEffect[CSound::E_SE_ECHO])
	{
		m_xapoEffect[CSound::E_SE_ECHO]->Release();
	}

	if (m_xapoEffect[CSound::E_SE_REVERBXAPO])
	{
		m_xapoEffect[CSound::E_SE_REVERBXAPO]->Release();
	}
}

/*
*/
void CSound::SetEffectChain()
{
	for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
	{
		m_hr = m_sourceVoice[i]->SetEffectChain(&m_effectChain);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteError(true, "CSound::SetEffectChain::SetEffectChain:Effect:%s\n", m_wavLoader->m_name->m_text);
			
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::SetEffectChain::SetEffectChain:", m_hr);

			return;
		}
	}
}

/*
*/
void CSound::SetEffectParameters()
{
	for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
	{
		m_hr = m_sourceVoice[i]->SetEffectParameters(CSound::E_SE_REVERB, &m_reverbParameters, sizeof(XAUDIO2FX_REVERB_PARAMETERS), XAUDIO2_COMMIT_NOW);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteError(true, "CSound::SetEffectParameters:CSound::SE_REVERB:Effect:%s\n", m_wavLoader->m_name->m_text);
			
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::SetEffectParameters:CSound::SE_REVERB:", m_hr);
		}

		m_hr = m_sourceVoice[i]->SetEffectParameters(CSound::E_SE_ECHO, &m_xapoEcho, sizeof(FXECHO_PARAMETERS), XAUDIO2_COMMIT_NOW);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteError(true, "CSound::SetEffectParameters:CSound::SE_ECHO:Effect:%s\n", m_wavLoader->m_name->m_text);
			
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::SetEffectParameters:CSound::SE_ECHO:", m_hr);
		}

		m_hr = m_sourceVoice[i]->SetEffectParameters(CSound::E_SE_REVERBXAPO, &m_xapoReverb, sizeof(FXREVERB_PARAMETERS), XAUDIO2_COMMIT_NOW);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteError(true, "CSound::SetEffectParameters:CSound::SE_REVERBXAPO:Effect:%s\n", m_wavLoader->m_name->m_text);
			
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::SetEffectParameters:CSound::SE_REVERBXAPO:", m_hr);
		}
	}
}

/*
*/
void CSound::Shutdown()
{
	for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
	{
		if (m_sourceVoice[i])
		{
			m_sourceVoice[i]->FlushSourceBuffers();

			m_sourceVoice[i]->DestroyVoice();
		}
	}
}

/*
*/
void CSound::StartSound()
{
	for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
	{
		if (m_sourceVoice[i])
		{
			m_sourceVoice[i]->GetState(&m_voiceState);

			if (m_voiceState.BuffersQueued == 0)
			{
				m_hr = m_sourceVoice[i]->SubmitSourceBuffer(&m_buffer);

				if (m_hr != S_OK)
				{
					m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::StartSound::SubmitSourceBuffer:", m_hr);
				}

				m_hr = m_sourceVoice[i]->Start(0);

				if (m_hr != S_OK)
				{
					m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::StartSound::Start:", m_hr);
				}

				break;
			}
		}
	}
}

/*
*/
void CSound::StopSound()
{
	for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
	{
		if (m_sourceVoice[i])
		{
			m_hr = m_sourceVoice[i]->Stop(0);

			if (m_hr != S_OK)
			{
				m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::StopSound::Stop:", m_hr);
			}
		}
	}
}

/*
*/
void CSound::ToggleEffect(BYTE soundEffect)
{
	if (m_isOn[soundEffect])
	{
		m_isOn[soundEffect] = false;

		for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
		{
			m_hr = m_sourceVoice[i]->DisableEffect(soundEffect);

			if (m_hr != S_OK)
			{
				m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::ToggleEffect::DisableEffect:", m_hr);

				return;
			}
		}
	}
	else
	{
		m_isOn[soundEffect] = true;

		for (int32_t i = 0; i < CSound::E_SOURCE_VOICE_COUNT; i++)
		{
			m_hr = m_sourceVoice[i]->EnableEffect(soundEffect);

			if (m_hr != S_OK)
			{
				m_errorLog->WriteXAudio2ErrorMessage(true, "CSound::ToggleEffect::EnableEffect:", m_hr);

				return;
			}
		}
	}
}