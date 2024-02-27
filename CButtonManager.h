#pragma once

#include "framework.h"

#include "CButton.h"
#include "CColor.h"
#include "CFontManager.h"
#include "CErrorLog.h"
#include "CLinkList.h"
#include "CMouseDevice.h"
#include "CShaderManager.h"
#include "CSoundManager.h"
#include "CVideoDevice.h"

class CButtonManager
{
public:

	CLinkList<CButton>* m_buttons;

	CButtonManager();
	CButtonManager(CVideoDevice* videoDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager, CShaderManager* shaderManager,
		CSoundManager* soundManager, CColor* color);
	~CButtonManager();

	CButton* Create(const char* name, CTexture* image, const char* fontName, CVertex2 size, CVertex2 position, BYTE audience, BYTE type);
	void Delete(const char* name);
	CButton* Get(const char* name);

private:

	CColor* m_color;
	CFontManager* m_fontManager;
	CErrorLog* m_errorLog;
	CMouseDevice* m_mouseDevice;
	CShaderManager* m_shaderManager;
	CSoundManager* m_soundManager;
	CVideoDevice* m_videoDevice;
};

