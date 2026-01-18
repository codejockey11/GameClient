#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"

#include "CButton.h"
#include "CConstantBuffer.h"
#include "CFontManager.h"
#include "CGraphicsAdapter.h"
#include "CMouseDevice.h"
#include "CPipelineState.h"
#include "CSoundDevice.h"

class CButtonManager
{
public:

	CButton* m_button;
	CConstantBuffer* m_imageWvp;
	CErrorLog* m_errorLog;
	CFontManager* m_fontManager;
	CGraphicsAdapter* m_graphicsAdapter;
	CList* m_buttons;
	CListNode* m_buttonNode;
	CMouseDevice* m_mouseDevice;
	CPipelineState* m_pipelineState;
	CSoundDevice* m_soundDevice;

	CButtonManager();
	CButtonManager(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CMouseDevice* mouseDevice, CErrorLog* errorLog, CConstantBuffer* imageWvp,
		CFontManager* fontManager, CPipelineState* pipelineState);
	~CButtonManager();

	CButton* Create(int32_t listGroup, const char* name, CFont* font, CWavLoader* soundOn, CWavLoader* soundOff, CWavLoader* soundClicked, CTexture* image,
		CVec2f position, CVec2f size, BYTE audience, BYTE type);
	void Delete(const char* name);
	CButton* Get(const char* name);
};