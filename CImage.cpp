#include "CImage.h"

/*
*/
CImage::CImage()
{
	memset(this, 0x00, sizeof(CImage));
}

/*
*/
CImage::CImage(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CTexture* texture, CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CImage));

	m_graphicsAdapter = graphicsAdapter;

	m_texture = texture;

	m_size = size;

	m_position = position;
	
	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->Close();

	m_vs = shaderBinaryManager->Get("Image.vs");
	m_ps = shaderBinaryManager->Get("Image.ps");

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, errorLog, "Image");

	m_shader->AllocateConstantBuffers(1);

	m_shader->CreateConstantBuffer(m_shader->m_b[0], sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4), m_commandList);

	m_shader->SetTextureCount(1);

	m_shader->AllocateDescriptorRange(2);

	m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
	m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

	m_shader->AllocateRootParameter();

	m_shader->AllocatePipelineState(false, VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

	m_shader->BindConstantBuffers();

	m_shader->BindResource(m_shader->m_t[0], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

	float xScale = (m_graphicsAdapter->m_swapChainViewport->m_viewport.Width > 0) ?
		2.0f / m_graphicsAdapter->m_swapChainViewport->m_viewport.Width : 0.0f;
	
	float yScale = (m_graphicsAdapter->m_swapChainViewport->m_viewport.Height > 0) ?
		2.0f / m_graphicsAdapter->m_swapChainViewport->m_viewport.Height : 0.0f;

	m_wvpMat = XMFLOAT4X4
	(
		xScale, 0, 0, 0,
		0, -yScale, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	);

	m_highlight = 1.0f;

	// vertex uv draw order
	// 0---1
	// | \ |
	// 3---2
	m_vertices[0].m_p.x = 0.0f;
	m_vertices[0].m_p.y = 0.0f;

	m_vertices[0].m_uv.x = 0.0f;
	m_vertices[0].m_uv.y = 0.0f;

	m_vertices[1].m_p.x = size.m_p.x;
	m_vertices[1].m_p.y = 0.0f;

	m_vertices[1].m_uv.x = 1.0f;
	m_vertices[1].m_uv.y = 0.0f;

	m_vertices[2].m_p.x = size.m_p.x;
	m_vertices[2].m_p.y = size.m_p.y;

	m_vertices[2].m_uv.x = 1.0f;
	m_vertices[2].m_uv.y = 1.0f;

	m_vertices[3].m_p.x = 0.0f;
	m_vertices[3].m_p.y = size.m_p.y;

	m_vertices[3].m_uv.x = 0.0f;
	m_vertices[3].m_uv.y = 1.0f;

	m_vertexBuffer = new CVertexBuffer();

	m_vertexBuffer->Constructor(m_graphicsAdapter, errorLog, NULL, VertexType::E_VT_VERTEXT, 4, (void*)m_vertices);

	m_vertexBuffer->SetCommandList(m_commandList);

	DWORD index[6] = {0, 1, 2, 0, 2, 3};

	/*
	index[0] = 0;
	index[1] = 1;
	index[2] = 2;

	index[3] = 0;
	index[4] = 2;
	index[5] = 3;
	*/
	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, errorLog, 6, index);

	m_indexBuffer->SetCommandList(m_commandList);

	CImage::UpdatePosition();
}

/*
*/
CImage::~CImage()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;

	m_commandList.Reset();

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}

	delete m_shader;
}

/*
*/
void CImage::Record()
{
	m_commandAllocators[m_graphicsAdapter->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_graphicsAdapter->m_backbufferIndex].Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_graphicsAdapter->m_swapChainViewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &m_graphicsAdapter->m_swapChainViewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&m_graphicsAdapter->m_swapChainRenderTargets[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		false,
		nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetGraphicsRootSignature(m_shader->m_rootSignature->m_signature.Get());

	m_commandList->SetPipelineState(m_shader->m_pipelineState->m_pipelineState.Get());

	m_shader->m_constantBuffer[m_shader->m_b[0]].Reset();
	
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_wvpMat);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_highlight);
	
	m_shader->UpdateConstantBuffers();

	m_commandList->SetDescriptorHeaps(1, m_shader->m_heap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

	m_indexBuffer->Draw();

	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);
}

/*
*/
void CImage::UpdatePosition()
{
	m_vertices[0].m_p.x = m_position.m_p.x;
	m_vertices[1].m_p.x = m_position.m_p.x + m_size.m_p.x;
	m_vertices[2].m_p.x = m_position.m_p.x + m_size.m_p.x;
	m_vertices[3].m_p.x = m_position.m_p.x;

	m_vertices[0].m_p.y = m_position.m_p.y;
	m_vertices[1].m_p.y = m_position.m_p.y;
	m_vertices[2].m_p.y = m_position.m_p.y + m_size.m_p.y;
	m_vertices[3].m_p.y = m_position.m_p.y + m_size.m_p.y;

	if ((m_vertices[1].m_p.x < 0.0f) || (m_vertices[3].m_p.y < 0.0f) ||
		(m_vertices[0].m_p.x > m_graphicsAdapter->m_width) ||
		(m_vertices[1].m_p.y > m_graphicsAdapter->m_height))
	{
		return;
	}

	m_vertexBuffer->LoadBuffer((void*)m_vertices);
}