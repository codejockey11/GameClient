#include "CTextureManager.h"

/*
*/
CTextureManager::CTextureManager()
{
	memset(this, 0x00, sizeof(CTextureManager));
}

/*
*/
CTextureManager::CTextureManager(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, const char* defaultTexture)
{
	memset(this, 0x00, sizeof(CTextureManager));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_shaderDirectory = new CString(m_local->m_installPath->m_text);

	m_shaderDirectory->Append("shaders/");

	m_filename = new CString(m_shaderDirectory->m_text);

	m_filename->Append("mipMap.cp");

	// Can't generate mip maps past 6.4 as it's probably a needed driver upgrade which I can't do with the hardware I am working with
	m_computeShader = new CShaderBinary(m_errorLog, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4, "main", "cs_6_4", m_local->m_installPath->m_text, m_shaderDirectory->m_text,
		"mipMap.cp", m_filename->m_text);

	SAFE_DELETE(m_filename);

	m_textures = new CList();

	m_defaultTexture = CTextureManager::Create(defaultTexture);
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
	m_errorLog->WriteError(true, "CTextureManager::Create:%s\n", name);

	m_texture = CTextureManager::Get(name);

	if (m_texture)
	{
		m_errorLog->WriteError(true, "CTextureManager::Create:Returned\n");

		return m_texture;
	}

	m_texture = new CTexture(m_graphicsAdapter, m_errorLog, m_computeShader, name);

	m_filename = new CString(m_local->m_installPath->m_text);

	m_filename->Append("main/");
	m_filename->Append(name);

	m_texture->CreateFromFilename(m_filename->m_text);

	SAFE_DELETE(m_filename);

	if (m_texture->m_isInitialized)
	{
		m_textures->Add(m_texture, name);

		m_errorLog->WriteError(true, "CTextureManager::Create:Completed\n");

		return m_texture;
	}

	SAFE_DELETE(m_texture);

	m_errorLog->WriteError(true, "CTextureManager::Create:Failed\n");

	return m_defaultTexture;
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