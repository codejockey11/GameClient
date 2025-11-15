#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec2f.h"

#include "CFont.h"
#include "CGraphicsAdapter.h"
#include "CImage.h"
#include "CMouseDevice.h"
#include "CShaderBinaryManager.h"

class CTextInput
{
public:

	bool m_isActive;
	bool m_isMouseOver;

	CErrorLog* m_errorLog;
	CFont* m_font;
	CGraphicsAdapter* m_graphicsAdapter;
	CImage* m_image;
	CMouseDevice* m_mouseDevice;
	CString* m_value;
	CVec2f m_position;
	CVec2f m_size;

	int m_cursorLocation;

	CTextInput();
	CTextInput(CGraphicsAdapter* graphicsAdapter, CMouseDevice* mouseDevice, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, int bundle, CFont* font,
		CTexture* texture, CVec2f position, CVec2f size);
	~CTextInput();

	void Active();
	void AddChar(const char c);
	void Backspace();
	void CheckMouseOver();
	void Delete();
	void DisplayText();
	void Inactive();
	void Record();
	void Reset();
};