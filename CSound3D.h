#pragma once

#include "framework.h"

#include "CErrorLog.h"
#include "CSoundDevice.h"
#include "CString.h"
#include "CWavLoader.h"

class CSound3D
{
public:

	CString* m_filename;
	
	X3DAUDIO_CONE m_emitterCone;
	X3DAUDIO_DSP_SETTINGS m_dspSettings;
	X3DAUDIO_EMITTER m_emitter;
	X3DAUDIO_LISTENER m_listener;
	X3DAUDIO_VECTOR m_emitterPos;
	X3DAUDIO_VECTOR m_listenerPos;
	
	bool m_useInnerRadius;
	bool m_useListenerCone;
	bool m_useRedirectToLFE;
	
	float m_listenerAngle;

	CSound3D();
	CSound3D(CErrorLog* errorLog, CSoundDevice* soundDevice, const char* filename, bool loop, X3DAUDIO_VECTOR* position, float range);
	~CSound3D();

	void SetListener(X3DAUDIO_VECTOR* position, X3DAUDIO_VECTOR* look, X3DAUDIO_VECTOR* up);
	void Shutdown();
	void StartSound();
	void StopSound();

private:

	CErrorLog* m_errorLog;
	CSoundDevice* m_soundDevice;
	CWavLoader* m_wavLoader;
	
	DWORD m_calcFlags;
	
	FLOAT32* m_matrix;
	
	IXAudio2SourceVoice* m_sourceVoice;
	
	X3DAUDIO_VECTOR m_distance;
	
	XAUDIO2_BUFFER m_buffer;
	
	float m_range;
	
	int m_count;
};