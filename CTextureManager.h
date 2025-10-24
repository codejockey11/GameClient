#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"

#include "CGraphicsAdapter.h"
#include "CShaderBinary.h"
#include "CTexture.h"

class CTextureManager
{
public:

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLinkList<CTexture>* m_textures;
	CLinkListNode<CTexture>* m_textureNode;
	CLocal* m_local;
	CShaderBinary* m_computeShader;
	CTexture* m_texture;

	CTextureManager();
	CTextureManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local);
	~CTextureManager();

	CTexture* Create(const char* name);
	void Delete(const char* name);
	CTexture* Get(const char* name);
};