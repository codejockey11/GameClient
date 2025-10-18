#pragma once

#include "framework.h"

#include "../GameCommon/CColor.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"

#include "CButton.h"
#include "CFontManager.h"
#include "CGraphicsAdapter.h"
#include "CMouseDevice.h"
#include "CSoundDevice.h"
#include "CShaderBinaryManager.h"

class CButtonManager
{
public:

	CLinkList<CButton>* m_buttons;

	CButtonManager();
	CButtonManager(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager, CShaderBinaryManager* shaderBinaryManager);
	~CButtonManager();

	CButton* Create(const char* name, CFont* font, CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image, CVec2f position, CVec2f size, BYTE audience, BYTE type);
	void Delete(const char* name);
	CButton* Get(const char* name);

private:

	CColor* m_color;
	CFontManager* m_fontManager;
	CErrorLog* m_errorLog;
	CMouseDevice* m_mouseDevice;
	CGraphicsAdapter* m_graphicsAdapter;
	CSoundDevice* m_soundDevice;
	CShaderBinaryManager* m_shaderBinaryManager;
};