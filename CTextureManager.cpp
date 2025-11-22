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

	m_textures = new CList();
}

/*
*/
CTextureManager::~CTextureManager()
{
	m_textureNode = m_textures->m_list;

	while ((m_textureNode) && (m_textureNode->m_object))
	{
		m_texture = (CTexture*)m_textureNode->m_object;

		SAFE_DELETE(m_texture);

		m_textureNode = m_textures->Delete(m_textureNode);
	}

	SAFE_DELETE(m_textures);
	SAFE_DELETE(m_computeShader);
}

/*
*/
CTexture* CTextureManager::Create(const char* name)
{
	m_texture = CTextureManager::Get(name);

	if (m_texture)
	{
		return m_texture;
	}

	m_texture = new CTexture(m_graphicsAdapter, m_errorLog, m_local, m_computeShader, name);

	m_texture->CreateFromFilename();

	if (m_texture->m_isInitialized)
	{
		m_textures->Add(m_texture, name);

		return m_texture;
	}

	SAFE_DELETE(m_texture);

	m_errorLog->WriteError(true, "CTextureManager::Create:%s\n", name);

	return nullptr;
}

/*
*/
void CTextureManager::Delete(const char* name)
{
	m_textureNode = m_textures->Search(name);

	if (m_textureNode)
	{
		m_texture = (CTexture*)m_textureNode->m_object;

		SAFE_DELETE(m_texture);

		m_textures->Delete(m_textureNode);
	}
}

/*
*/
CTexture* CTextureManager::Get(const char* name)
{
	m_textureNode = m_textures->Search(name);

	if (m_textureNode)
	{
		return (CTexture*)m_textureNode->m_object;
	}

	return nullptr;
}