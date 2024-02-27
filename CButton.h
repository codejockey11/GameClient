#pragma once

#include "framework.h"

#include "CFontManager.h"
#include "CErrorLog.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CNetwork.h"
#include "CSoundManager.h"
#include "CVideoDevice.h"

class CButton
{
public:

	CColor* m_color;
	CFont* m_font;
	CImage* m_image;
	CNetwork* m_network;
	CString* m_name;
	CVertex2 m_position;
	CVertex2 m_size;
	
	bool m_isInitialized;
	bool m_isMouseOver;

	CButton();
	CButton(CVideoDevice* videoDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CFontManager* fontManager, CShaderManager* shaderManager,
		CSoundManager* soundManager, CColor* color, const char* name, CTexture* image, const char* fontName, CVertex2 size, CVertex2 position, BYTE audience, BYTE type);
	~CButton();

	void Active();
	void DisplayText();
	void Inactive();
	void Record();

private:

	CMouseDevice* m_mouseDevice;
	CSound* m_soundClicked;
	CSound* m_soundOff;
	CSound* m_soundOn;
	
	bool m_soundOffPlayed;
	bool m_soundOnPlayed;
};