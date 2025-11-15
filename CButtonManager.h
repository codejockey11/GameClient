#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"

#include "CButton.h"
#include "CFontManager.h"
#include "CGraphicsAdapter.h"
#include "CMouseDevice.h"
#include "CShaderBinaryManager.h"
#include "CSoundDevice.h"

class CButtonManager
{
public:

	CButton* m_button;
	CErrorLog* m_errorLog;
	CFontManager* m_fontManager;
	CGraphicsAdapter* m_graphicsAdapter;
	CList* m_buttons;
	CListNode* m_buttonNode;
	CMouseDevice* m_mouseDevice;
	CShaderBinaryManager* m_shaderBinaryManager;
	CSoundDevice* m_soundDevice;

	CButtonManager();
	CButtonManager(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager,
		CShaderBinaryManager* shaderBinaryManager);
	~CButtonManager();

	CButton* Create(int bundle, const char* name, CFont* font, CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image, CVec2f position, CVec2f size,
		BYTE audience, BYTE type);
	void Delete(const char* name);
	CButton* Get(const char* name);
};