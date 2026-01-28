#include "CSound3D.h"

/*
*/
CSound3D::CSound3D()
{
	memset(this, 0x00, sizeof(CSound3D));
}

/*
*/
CSound3D::CSound3D(CErrorLog* errorLog, CSoundDevice* soundDevice, CWavLoader* wavLoader, bool loop)
{
	memset(this, 0x00, sizeof(CSound3D));

	m_isInitialized = false;

	m_errorLog = errorLog;

	m_soundDevice = soundDevice;

	m_wavLoader = wavLoader;

	for (int32_t i = 0; i < CSound3D::E_SOURCE_VOICE_COUNT; i++)
	{
		m_hr = m_soundDevice->m_xAudio2->CreateSourceVoice(&m_sourceVoice[i], &m_wavLoader->m_wfx);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::CSound3D::CreateSourceVoice:", m_hr);

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

	m_emitter.ChannelCount = 1;
	m_emitter.CurveDistanceScaler = 1.0f;

	m_calcFlags = X3DAUDIO_CALCULATE_MATRIX;

	m_matrix = new float[2]();

	memset((void*)m_matrix, 0x00, sizeof(float) * 2);

	m_dspSettings.SrcChannelCount = m_wavLoader->m_wfx.nChannels;
	m_dspSettings.DstChannelCount = 2;
	m_dspSettings.pMatrixCoefficients = m_matrix;

	m_isInitialized = true;
}

/*
*/
CSound3D::~CSound3D()
{
	CSound3D::StopSound();
	CSound3D::Shutdown();

	SAFE_DELETE_ARRAY(m_matrix);
}

/*
*/
void CSound3D::SetListener(X3DAUDIO_VECTOR* position, X3DAUDIO_VECTOR* look, X3DAUDIO_VECTOR* up)
{
	memset((void*)&m_listener, 0x00, sizeof(X3DAUDIO_LISTENER));

	m_listener.Position = (*position);
	m_listener.OrientFront = (*look);
	m_listener.OrientTop = (*up);

	m_distance.x = m_listener.Position.x - m_emitter.Position.x;
	m_distance.y = m_listener.Position.y - m_emitter.Position.y;
	m_distance.z = m_listener.Position.z - m_emitter.Position.z;

	m_dir.m_p.x = m_distance.x;
	m_dir.m_p.y = m_distance.y;
	m_dir.m_p.z = m_distance.z;

	m_dist = m_dir;

	m_inRange = true;

	float t = m_dist.Length();

	if (t == 0)
	{
		return;
	}

	m_dir.Normalize();

	t = m_dist.Length();

	if (t > m_range)
	{
		m_inRange = false;

		return;
	}

	t = t / m_range;

	m_dir *= (t * 16.0f);

	m_listener.Position.x = m_emitter.Position.x + m_dir.m_p.x;
	m_listener.Position.y = m_emitter.Position.y + m_dir.m_p.y;
	m_listener.Position.z = m_emitter.Position.z + m_dir.m_p.z;
}

/*
*/
void CSound3D::SetPosition(CVec3f* p)
{
	m_emitter.Position.x = p->m_p.x;
	m_emitter.Position.y = p->m_p.y;
	m_emitter.Position.z = p->m_p.z;
}

/*
*/
void CSound3D::SetPosition(float x, float y, float z)
{
	m_emitter.Position.x = x;
	m_emitter.Position.y = y;
	m_emitter.Position.z = z;
}

/*
*/
void CSound3D::SetPosition(XMFLOAT3* p)
{
	m_emitter.Position.x = p->x;
	m_emitter.Position.y = p->y;
	m_emitter.Position.z = p->z;
}

/*
*/
void CSound3D::SetPosition(XMFLOAT4* p)
{
	m_emitter.Position.x = p->x;
	m_emitter.Position.y = p->y;
	m_emitter.Position.z = p->z;
}

/*
*/
void CSound3D::SetRange(float r)
{
	m_range = r;
}

/*
*/
void CSound3D::Shutdown()
{
	for (int32_t i = 0; i < CSound3D::E_SOURCE_VOICE_COUNT; i++)
	{
		m_sourceVoice[i]->FlushSourceBuffers();

		m_sourceVoice[i]->DestroyVoice();
	}
}

/*
*/
void CSound3D::StartSound()
{
	if (!m_inRange)
	{
		return;
	}

	X3DAudioCalculate(m_soundDevice->m_3DAudio, &m_listener, &m_emitter, m_calcFlags, &m_dspSettings);

	for (int32_t i = 0; i < CSound3D::E_SOURCE_VOICE_COUNT; i++)
	{
		m_sourceVoice[i]->GetState(&m_voiceState);

		if (m_voiceState.BuffersQueued == 1)
		{
		}
		else
		{
			m_sourceVoice[i]->SetOutputMatrix(m_soundDevice->m_masteringVoice, m_wavLoader->m_wfx.nChannels, 2, m_dspSettings.pMatrixCoefficients, 0);

			m_hr = m_sourceVoice[i]->SubmitSourceBuffer(&m_buffer);

			if (m_hr != S_OK)
			{
				m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::StartSound::SubmitSourceBuffer:", m_hr);

				return;
			}

			m_hr = m_sourceVoice[i]->Start(0);

			if (m_hr != S_OK)
			{
				m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::StartSound::Start:", m_hr);

				return;
			}

			break;
		}
	}
}

/*
*/
void CSound3D::StopSound()
{
	for (int32_t i = 0; i < CSound3D::E_SOURCE_VOICE_COUNT; i++)
	{
		m_hr = m_sourceVoice[i]->Stop(0);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::StopSound::Stop:", m_hr);

			return;
		}
	}
}