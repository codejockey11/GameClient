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

	CString* shaderDirectory = new CString(m_local->m_installPath->m_text);

	shaderDirectory->Append("shaders/");

	CString* filename = new CString(shaderDirectory->m_text);

	filename->Append("mipMap.cp");

	// Can't generate mip maps past 6.4 as it's probably a needed driver upgrade which I can't do with the hardware I am working with
	m_computeShader = new CShaderBinary(m_errorLog, D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4, "main", "cs_6_4", m_local->m_installPath->m_text, shaderDirectory->m_text,
		"mipMap.cp", filename->m_text);

	SAFE_DELETE(filename);
	
	SAFE_DELETE(shaderDirectory);

	m_textures = new CList();

	m_defaultTexture = CTextureManager::Create(defaultTexture, 0);
}

/*
*/
CTextureManager::~CTextureManager()
{
	CListNode* node = m_textures->m_list;

	while ((node) && (node->m_object))
	{
		CTexture* texture = (CTexture*)node->m_object;

		SAFE_DELETE(texture);

		node = m_textures->Delete(node);
	}

	SAFE_DELETE(m_textures);
	SAFE_DELETE(m_computeShader);
}

/*
*/
CTexture* CTextureManager::Create(const char* name, int32_t mipLevel)
{
	CTexture* texture = CTextureManager::Get(name);

	if (texture != m_defaultTexture)
	{
		return texture;
	}

	m_errorLog->WriteError(true, "CTextureManager::Create:%s\n", name);

	texture = new CTexture(m_graphicsAdapter, m_errorLog, m_computeShader, name);

	CString* filename = new CString(m_local->m_installPath->m_text);

	filename->Append("main/");
	filename->Append(name);

	texture->CreateFromFilename(filename->m_text, mipLevel);

	SAFE_DELETE(filename);

	if (texture->m_isInitialized)
	{
		m_textures->Add(texture, name);

		return texture;
	}

	SAFE_DELETE(texture);

	m_errorLog->WriteError(true, "CTextureManager::Create:Failed\n");

	return m_defaultTexture;
}

/*
*/
void CTextureManager::Delete(const char* name)
{
	CListNode* node = m_textures->Search(name);

	if ((node) && (node->m_object))
	{
		CTexture* texture = (CTexture*)node->m_object;

		SAFE_DELETE(texture);

		m_textures->Delete(node);
	}
}

/*
*/
CTexture* CTextureManager::Get(const char* name)
{
	CListNode* node = m_textures->Search(name);

	if ((node) && (node->m_object))
	{
		return (CTexture*)node->m_object;
	}

	return m_defaultTexture;
}