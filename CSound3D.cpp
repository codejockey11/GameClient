#include "CSound3D.h"

/*
*/
CSound3D::CSound3D()
{
	memset(this, 0x00, sizeof(CSound3D));
}

/*
*/
CSound3D::CSound3D(CErrorLog* errorLog, CSoundDevice* soundDevice, const char* filename, bool loop, X3DAUDIO_VECTOR* position, float range)
{
	memset(this, 0x00, sizeof(CSound3D));

	m_errorLog = errorLog;
	m_soundDevice = soundDevice;
	m_range = range;

	m_filename = new CString(filename);

	m_wavLoader = new CWavLoader(m_errorLog, filename);

	HRESULT hr = m_soundDevice->m_xAudio2->CreateSourceVoice(&m_sourceVoice, &m_wavLoader->m_wfx);

	if (hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::CSound3D::CreateSourceVoice:", hr);

		return;
	}

	m_buffer.AudioBytes = m_wavLoader->m_header.m_subchunk2Size;
	m_buffer.Flags = XAUDIO2_END_OF_STREAM;
	m_buffer.pAudioData = m_wavLoader->m_data;

	if (loop)
	{
		m_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	m_emitter.Position.x = position->x;
	m_emitter.Position.y = position->y;
	m_emitter.Position.z = position->z;

	m_emitter.ChannelCount = 1;
	m_emitter.CurveDistanceScaler = 1.0f;

	m_calcFlags = X3DAUDIO_CALCULATE_MATRIX;

	// deviceDetails overly complicated just use 2 channels
	m_matrix = new FLOAT32[2];

	memset((void*)m_matrix, 0x00, 2 * sizeof(FLOAT32));

	m_dspSettings.SrcChannelCount = m_wavLoader->m_wfx.nChannels;
	m_dspSettings.DstChannelCount = 2;
	m_dspSettings.pMatrixCoefficients = m_matrix;
}

/*
*/
CSound3D::~CSound3D()
{
	delete m_filename;
	delete m_wavLoader;

	m_sourceVoice->Stop(0);

	m_sourceVoice->FlushSourceBuffers();

	m_sourceVoice->DestroyVoice();
}

/*
*/
void CSound3D::SetListener(X3DAUDIO_VECTOR* position, X3DAUDIO_VECTOR* look, X3DAUDIO_VECTOR* up)
{
	memset((void*)&m_listener, 0x00, sizeof(X3DAUDIO_LISTENER));

	m_listener.Position = *position;
	m_listener.OrientFront = *look;
	m_listener.OrientTop = *up;

	m_distance.x = m_listener.Position.x - m_emitter.Position.x;
	m_distance.y = m_listener.Position.y - m_emitter.Position.y;
	m_distance.z = m_listener.Position.z - m_emitter.Position.z;

	XMVECTOR res = XMVector3Length(XMLoadFloat3(&m_distance));

	if (XMVectorGetX(res) > m_range)
	{
		m_count = 0;

		CSound3D::StopSound();
	}
	else if (m_count == 0)
	{
		m_count = 1;

		CSound3D::StartSound();
	}

	X3DAudioCalculate(m_soundDevice->m_3DAudio, &m_listener, &m_emitter, m_calcFlags, &m_dspSettings);

	// just going to code 2 channels
	m_sourceVoice->SetOutputMatrix(m_soundDevice->m_masteringVoice, m_wavLoader->m_wfx.nChannels, 2, m_dspSettings.pMatrixCoefficients, 0);
}

/*
*/
void CSound3D::Shutdown()
{
	m_sourceVoice->FlushSourceBuffers();

	m_sourceVoice->DestroyVoice();
}

/*
*/
void CSound3D::StartSound()
{
	HRESULT hr = m_sourceVoice->SubmitSourceBuffer(&m_buffer);

	if (hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::StartSound::SubmitSourceBuffer:", hr);

		return;
	}

	hr = m_sourceVoice->Start(0);

	if (hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::StartSound::Start:", hr);

		return;
	}
}

/*
*/
void CSound3D::StopSound()
{
	HRESULT hr = m_sourceVoice->Stop(0);

	if (hr != S_OK)
	{
		m_errorLog->WriteXAudio2ErrorMessage(true, "CSound3D::StopSound::Stop:", hr);

		return;
	}
}