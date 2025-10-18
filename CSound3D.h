#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec3f.h"

#include "CSoundDevice.h"
#include "CWavLoader.h"

class CSound3D
{
public:
	
	enum
	{
		E_SOURCE_VOICE_COUNT = 16
	};

	bool m_initialized;
	bool m_useInnerRadius;
	bool m_useListenerCone;
	bool m_useRedirectToLFE;

	CWavLoader* m_wavLoader;

	float m_listenerAngle;

	X3DAUDIO_CONE m_emitterCone;
	X3DAUDIO_DSP_SETTINGS m_dspSettings;
	X3DAUDIO_EMITTER m_emitter;
	X3DAUDIO_LISTENER m_listener;
	X3DAUDIO_VECTOR m_emitterPos;
	X3DAUDIO_VECTOR m_listenerPos;

	CSound3D();
	CSound3D(CErrorLog* errorLog, CSoundDevice* soundDevice, CWavLoader* wavLoader, bool loop);
	~CSound3D();

	void SetListener(X3DAUDIO_VECTOR* position, X3DAUDIO_VECTOR* look, X3DAUDIO_VECTOR* up);
	void SetPosition(CVec3f* p);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3* p);
	void SetPosition(XMFLOAT4* p);
	void SetRange(float r);
	void Shutdown();
	void StartSound();
	void StopSound();

private:

	CErrorLog* m_errorLog;
	CSoundDevice* m_soundDevice;
	
	DWORD m_calcFlags;
	
	FLOAT32* m_matrix;
	
	IXAudio2SourceVoice* m_sourceVoice[CSound3D::E_SOURCE_VOICE_COUNT];
	XAUDIO2_VOICE_STATE m_voiceState;

	X3DAUDIO_VECTOR m_distance;
	CVec3f m_dir;
	CVec3f m_dist;
	bool m_inRange;
	
	XAUDIO2_BUFFER m_buffer;
	
	float m_range;
	
	int m_count;
};