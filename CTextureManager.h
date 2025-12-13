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
	CListNode* m_textureNode;
	CLocal* m_local;
	CShaderBinary* m_computeShader;
	CString* m_shaderDirectory;
	CString* m_filename;
	CTexture* m_defaultTexture;
	CTexture* m_texture;

	CTextureManager();
	CTextureManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, const char* defaultTexture);
	~CTextureManager();

	CTexture* Create(const char* name);
	void Delete(const char* name);
	CTexture* Get(const char* name);
};