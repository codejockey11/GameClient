#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CLocal.h"

#include "CGraphicsAdapter.h"
#include "CShaderBinary.h"
#include "CTexture.h"

class CTextureManager
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CList* m_textures;
	CLocal* m_local;
	CShaderBinary* m_computeShader;
	CTexture* m_defaultTexture;

	CTextureManager();
	CTextureManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, const char* defaultTexture);
	~CTextureManager();

	CTexture* Create(const char* name, int32_t mipLevel);
	void Delete(const char* name);
	CTexture* Get(const char* name);
};