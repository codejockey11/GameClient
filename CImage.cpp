#include "CImage.h"

/*
*/
CImage::CImage()
{
	memset(this, 0x00, sizeof(CImage));
}

/*
*/
CImage::CImage(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CPipelineState* pipelineState, int32_t listGroup, CTexture* texture, CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(CImage));

	m_graphicsAdapter = graphicsAdapter;

	m_pipelineState = pipelineState;

	m_texture = texture;

	m_listGroup = listGroup;

	m_position = position;

	m_size = size;

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, errorLog);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(L"CImage::CImage");

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, errorLog, "Image");

	m_shader->SetConstantBufferCount(1);
	m_shader->SetTextureCount(1);

	m_constantBuffer = new CConstantBuffer();

	m_constantBuffer->Constructor(m_graphicsAdapter, errorLog, sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4));

	m_shaderHeap = m_shader->AllocateHeap();

	m_shaderHeap->BindResource(0, m_constantBuffer->m_buffer.Get(), &m_constantBuffer->m_srvDesc);

	m_shaderHeap->BindResource(m_shader->m_t[0], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

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

	m_vertexBuffer->Constructor(m_graphicsAdapter, errorLog, NULL, CPipelineState::VertexType::E_VT_VERTEXT, 4, (void*)m_vertices);

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
	SAFE_DELETE(m_shaderHeap);
	SAFE_DELETE(m_constantBuffer);
	SAFE_DELETE(m_shader);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
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

	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap->GetBaseDescriptor());

	CImage::UpdateVertexBuffer();

	m_indexBuffer->Record();

	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);
}

/*
*/
void CImage::RecordConstantBuffer()
{
	m_constantBuffer->Reset();

	m_constantBuffer->m_values->Append(m_graphicsAdapter->m_wvpMat);
	m_constantBuffer->m_values->Append(m_highlight);

	m_constantBuffer->Record();
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