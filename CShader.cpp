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
	delete m_pipelineState;
	delete m_rootSignature;
	delete m_rootParameter;
	delete m_descriptorRange;

	delete[] m_constantBuffer;
	delete[] m_t;
	delete[] m_b;

	delete m_heap;
	delete m_name;
}

/*
*/
void CShader::AllocateConstantBuffers(int count)
{
	m_constantBufferCount = count;

	m_constantBuffer = new CConstantBuffer[m_constantBufferCount]();

	m_b = new int[m_constantBufferCount]();

	for (int i = 0; i < m_constantBufferCount; i++)
	{
		m_b[i] = i;
	}
}

/*
*/
void CShader::AllocateDescriptorRange(int count)
{
	m_descriptorRange = new CDescriptorRange(count);

	m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_constantBufferCount);
	m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_textureCount);
}

/*
*/
void CShader::AllocateRootParameter()
{
	m_rootParameter = new CRootParameter(1);

	m_rootParameter->InitDescriptorTable(0, m_descriptorRange->m_count, m_descriptorRange->m_range);

	m_rootSignature = new CRootSignature(m_graphicsAdapter, m_errorLog);

	m_rootSignature->Create(m_rootParameter->m_count, m_rootParameter->m_rootParameter);

	m_heap = new CShaderHeap(m_graphicsAdapter, m_errorLog, m_descriptorRange->m_numberOfDescriptors);
}

/*
*/
void CShader::AllocatePipelineState(bool depthEnable, BYTE type, D3D12_CULL_MODE cullMode, CShaderBinary* vsBinary, CShaderBinary* psBinary)
{
	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, type, cullMode);

	m_pipelineState->m_desc.VS.pShaderBytecode = vsBinary->m_shader->GetBufferPointer();
	m_pipelineState->m_desc.VS.BytecodeLength = vsBinary->m_shader->GetBufferSize();

	m_pipelineState->m_desc.PS.pShaderBytecode = psBinary->m_shader->GetBufferPointer();
	m_pipelineState->m_desc.PS.BytecodeLength = psBinary->m_shader->GetBufferSize();

	m_pipelineState->Create(false, true, depthEnable, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_rootSignature);
}

/*
*/
void CShader::AllocatePipelineStateForShadowMap(BYTE type, CShaderBinary* vsBinary, CShaderBinary* psBinary)
{
	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, type, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->m_desc.VS.pShaderBytecode = vsBinary->m_shader->GetBufferPointer();
	m_pipelineState->m_desc.VS.BytecodeLength = vsBinary->m_shader->GetBufferSize();

	m_pipelineState->m_desc.PS.pShaderBytecode = psBinary->m_shader->GetBufferPointer();
	m_pipelineState->m_desc.PS.BytecodeLength = psBinary->m_shader->GetBufferSize();

	m_pipelineState->CreateShadowMap(D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_rootSignature);
}

/*
*/
void CShader::BindConstantBuffers()
{
	for (int i = 0; i < m_constantBufferCount; i++)
	{
		CShader::BindResource(i, m_constantBuffer[i].m_buffer.Get(), &m_constantBuffer[i].m_srvDesc);
	}
}

/*
*/
void CShader::BindResource(UINT location, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* m_srvDesc)
{
	m_CPUDescriptorHandle = m_heap->m_CPUDescriptorHandle;

	m_CPUDescriptorHandle.ptr += ((SIZE_T)location * m_graphicsAdapter->m_descriptorHandleIncrementSize);

	m_graphicsAdapter->m_device->CreateShaderResourceView(resource, m_srvDesc, m_CPUDescriptorHandle);
}

/*
*/
void CShader::CreateConstantBuffer(int number, int size, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_constantBuffer[number].Constructor(m_graphicsAdapter, m_errorLog, size, commandList);
}

/*
*/
void CShader::SetTextureCount(int textureCount)
{
	m_textureCount = textureCount;

	m_t = new int[m_textureCount]();

	m_t[0] = m_constantBufferCount;

	for (int i = 1;i < m_textureCount; i++)
	{
		m_t[i] = m_t[i - 1] + 1;
	}
}

/*
*/
void CShader::UpdateConstantBuffers()
{
	for (int i = 0; i < m_constantBufferCount; i++)
	{
		m_constantBuffer[i].UpdateConstantBuffer();
	}
}