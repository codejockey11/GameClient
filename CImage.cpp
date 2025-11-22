#include "CImage.h"

/*
*/
CImage::CImage()
{
	memset(this, 0x00, sizeof(CImage));
}

/*
*/
CImage::CImage(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, int32_t bundle, CTexture* texture, CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CImage));

	m_graphicsAdapter = graphicsAdapter;

	m_texture = texture;

	m_bundle = bundle;

	m_position = position;

	m_size = size;

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->Close();

	m_vs = shaderBinaryManager->Get("image.vs");
	m_ps = shaderBinaryManager->Get("image.ps");
	
	m_pipelineState = new CPipelineState(m_graphicsAdapter, errorLog, VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->AllocateSignature(1, 1);

	m_pipelineState->SetPixelShader(m_ps);
	m_pipelineState->SetVertexShader(m_vs);

	m_pipelineState->Create(false, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_constantBuffer = new CConstantBuffer();

	m_constantBuffer->Constructor(m_graphicsAdapter, errorLog, m_commandList,
		sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4));

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, errorLog, "Image");

	m_shader->SetConstantBufferCount(1);
	m_shader->SetTextureCount(1);

	m_shader->AllocateHeap();

	m_shader->BindResource(0, m_constantBuffer->m_buffer.Get(), &m_constantBuffer->m_srvDesc);
	
	m_shader->BindResource(m_shader->m_t[0], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

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

	uint16_t index[6] = { 0, 1, 2, 0, 2, 3 };

	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, errorLog, 6, index);

	m_indexBuffer->SetCommandList(m_commandList);

	m_needsUpload = true;

	CImage::UpdateVertexBuffer();

	m_box = new CBoundBox(m_position.m_p.x, m_position.m_p.y, 0.0f, m_size.m_p.x, m_size.m_p.y, 0.0f);
}

/*
*/
CImage::~CImage()
{
	SAFE_DELETE(m_box);
	SAFE_DELETE(m_indexBuffer);
	SAFE_DELETE(m_vertexBuffer);
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_constantBuffer);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
	SAFE_DELETE(m_pipelineState);
}

/*
*/
void CImage::Move(float x, float y)
{
	m_position.m_p.x += x;
	m_position.m_p.y += y;

	m_box->Move(x, y, 0.0f);

	m_needsUpload = true;
}

/*
*/
void CImage::Record()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	m_commandList->RSSetViewports(1, m_graphicsAdapter->GetViewport());

	m_commandList->RSSetScissorRects(1, m_graphicsAdapter->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, m_graphicsAdapter->GetRenderTarget(), false, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());

	m_constantBuffer->Reset();

	m_constantBuffer->m_values->Append(m_graphicsAdapter->m_wvpMat);
	m_constantBuffer->m_values->Append(m_highlight);

	m_constantBuffer->UpdateBuffer();

	m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

	CImage::UpdateVertexBuffer();

	m_indexBuffer->Record();

	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);

	m_graphicsAdapter->BundleCommandList(m_bundle, m_commandList);
}

/*
*/
void CImage::SetPosition(CVec2f* position)
{
	m_position = *position;

	m_needsUpload = true;
}

/*
*/
void CImage::UpdateVertexBuffer()
{
	if (m_needsUpload)
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

		m_needsUpload = false;
	}
}