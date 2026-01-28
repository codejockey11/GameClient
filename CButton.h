#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CNetwork.h"

#include "CConstantBuffer.h"
#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CPipelineState.h"
#include "CSound.h"

class CButton
{
public:

	bool m_isInitialized;
	bool m_isMouseOver;
	bool m_soundOffPlayed;
	bool m_soundOnPlayed;

	CFont* m_font;
	CGraphicsAdapter* m_graphicsAdapter;
	CImage* m_image;
	CMouseDevice* m_mouseDevice;
	CNetwork* m_network;
	CSound* m_soundClicked;
	CSound* m_soundOff;
	CSound* m_soundOn;
	CString* m_name;
	CVec2f m_position;
	CVec2f m_size;

	CButton();
	CButton(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState,
		int32_t listGroup, const char* name, CFont* font, CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* texture, CVec2f position, CVec2f size,
		BYTE audience, BYTE type);
	~CButton();

	void Active();
	void CheckMouseOver();
	void DisplayText();
	void Inactive();
	void Move(float x, float y);
	void Record();
};