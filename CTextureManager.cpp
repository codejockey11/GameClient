#include "CTextureManager.h"

/*
*/
CTextureManager::CTextureManager()
{
	memset(this, 0x00, sizeof(CTextureManager));
}

/*
*/
CTextureManager::CTextureManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local)
{
	memset(this, 0x00, sizeof(CTextureManager));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	// Can't generate mip maps past 6.4 as it's probably a needed driver upgrade which I can't do with the hardware I am working with
	m_computeShader = new CShaderBinary(m_errorLog, m_local, "mipMap.cp", "main", "cs_6_4", D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4);

	m_textures = new CLinkList<CTexture>();
}

/*
*/
CTextureManager::~CTextureManager()
{
	delete m_textures;
	delete m_computeShader;
}

/*
*/
CTexture* CTextureManager::Create(const char* name)
{
	CTexture* texture = CTextureManager::Get(name);

	if (texture)
	{
		return texture;
	}

	texture = new CTexture(m_graphicsAdapter, m_errorLog, m_local, m_computeShader, name);

	texture->CreateFromFilename();

	if (texture->m_isInitialized)
	{
		m_textures->Add(texture, name);

		return texture;
	}

	delete texture;

	m_errorLog->WriteError(true, "CTextureManager::Create:%s\n", name);

	return nullptr;
}

/*
*/
CTexture* CTextureManager::Get(const char* name)
{
	CLinkListNode<CTexture>* lln = m_textures->Search(name);

	if (lln)
	{
		return lln->m_object;
	}

	return nullptr;
}

/*
*/
void CTextureManager::Delete(const char* name)
{
	CLinkListNode<CTexture>* lln = m_textures->Search(name);

	if (lln)
	{
		m_textures->Delete(lln);
	}
}