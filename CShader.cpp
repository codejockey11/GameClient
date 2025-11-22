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

	SAFE_DELETE(m_heap);
	SAFE_DELETE(m_name);
}

/*
*/
void CShader::AllocateHeap()
{
	m_heap = new CShaderHeap(m_graphicsAdapter, m_errorLog, m_constantBufferCount + m_textureCount);
}

/*
*/
void CShader::BindResource(int32_t location, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	m_CPUDescriptorHandle = m_heap->m_CPUDescriptorHandle;

	m_CPUDescriptorHandle.ptr += ((SIZE_T)location * m_graphicsAdapter->m_descriptorHandleIncrementSize);

	m_graphicsAdapter->m_device->CreateShaderResourceView(resource, srvDesc, m_CPUDescriptorHandle);
}

/*
*/
D3D12_GPU_DESCRIPTOR_HANDLE CShader::GetBaseDescriptor()
{
	return m_heap->m_heap->GetGPUDescriptorHandleForHeapStart();
}

/*
*/
ID3D12DescriptorHeap* const* CShader::GetDescriptorHeap()
{
	return m_heap->m_heap.GetAddressOf();
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