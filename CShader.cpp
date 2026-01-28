#include "CShader.h"

/*
*/
CShader::CShader()
{
	memset(this, 0x00, sizeof(CShader));
}

/*
*/
void CShader::Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, const char* name)
{
	memset(this, 0x00, sizeof(CShader));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_name = new CString(name);
}

/*
*/
CShader::~CShader()
{
	SAFE_DELETE_ARRAY(m_t);

	SAFE_DELETE(m_name);
}

/*
*/
CShaderHeap* CShader::AllocateHeap()
{
	return new CShaderHeap(m_graphicsAdapter, m_errorLog, m_constantBufferCount + m_textureCount);
}

/*
*/
void CShader::SetConstantBufferCount(int32_t constantBufferCount)
{
	m_constantBufferCount = constantBufferCount;
}

/*
*/
void CShader::SetTextureCount(int32_t textureCount)
{
	m_textureCount = textureCount;

	m_t = new int32_t[m_textureCount]();

	m_t[0] = m_constantBufferCount;

	for (int32_t i = 1; i < m_textureCount; i++)
	{
		m_t[i] = m_t[i - 1] + 1;
	}
}