#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CNetwork.h"

#include "CBoundBox.h"
#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CSound.h"
#include "CShaderBinaryManager.h"

class CButton
{
public:

	CBoundBox* m_box;
	CFont* m_font;
	CImage* m_image;
	CNetwork* m_network;
	CString* m_name;
	CVec2f m_position;
	CVec2f m_size;
	
	bool m_isInitialized;
	bool m_isMouseOver;

	CButton();
	CButton(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, const char* name, CFont* font,
		CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image, CVec2f position, CVec2f size, BYTE audience, BYTE type);
	~CButton();

	void Active();
	void CheckMouseOver();
	void DisplayText();
	void Inactive();
	void Record();

private:

	CMouseDevice* m_mouseDevice;
	CGraphicsAdapter* m_graphicsAdapter;
	
	CSound* m_soundClicked;
	CSound* m_soundOff;
	CSound* m_soundOn;
	
	bool m_soundOffPlayed;
	bool m_soundOnPlayed;
};