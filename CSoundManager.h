#pragma once

#include "framework.h"

#include "CErrorLog.h"
#include "CLinkList.h"
#include "CSound.h"
#include "CSoundDevice.h"

class CSoundManager
{
public:

	CSoundManager();
	CSoundManager(CSoundDevice* soundDevice, CErrorLog* errorLog);
	~CSoundManager();

	CSound* Create(const char* filename, BYTE loop);
	void Delete(const char* name);
	CSound* Get(const char* filename);

private:

	CErrorLog* m_errorLog;
	CLinkList<CSound>* m_sounds;
	CSoundDevice* m_soundDevice;
};