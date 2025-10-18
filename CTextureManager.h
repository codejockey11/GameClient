#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"

#include "CGraphicsAdapter.h"
#include "CTexture.h"

class CTextureManager
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLinkList<CTexture>* m_textures;
	CLocal* m_local;
	CShaderBinary* m_computeShader;

	CTextureManager();
	CTextureManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local);
	~CTextureManager();

	CTexture* Create(const char* name);
	void Delete(const char* name);
	CTexture* Get(const char* name);
};