#include "CGlyphText.h"

/*
*/
CGlyphText::CGlyphText()
{
	memset(this, 0x00, sizeof(CGlyphText));
}

/*
*/
CGlyphText::CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, CGlyphFixed* glyphFixed, int32_t listGroup,
	int32_t textLength, CVec2f position, float size)
{
	memset(this, 0x00, sizeof(CGlyphText));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_imageWvp = imageWvp;

	m_pipelineState = pipelineState;

	m_listGroup = listGroup;

	m_glyphVertices = glyphFixed->m_glyphVertices;

	m_text = new CString(textLength);

	m_position = position;

	m_size = size;

	m_glyphSize = glyphFixed->m_glyphHeight;

	m_sizeRatio = m_size / m_glyphSize;

	m_glyphs = glyphFixed->m_glyphs;

	CGlyphText::AllocateResources();
}

/*
*/
CGlyphText::CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, CGlyphVariable* glyphVariable, int32_t listGroup,
	int32_t textLength, CVec2f position, float size)
{
	memset(this, 0x00, sizeof(CGlyphText));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_imageWvp = imageWvp;

	m_pipelineState = pipelineState;

	m_listGroup = listGroup;

	m_glyphVertices = glyphVariable->m_glyphVertices;

	m_text = new CString(textLength);

	m_position = position;

	m_size = size;

	m_glyphSize = glyphVariable->m_glyphHeight;

	m_sizeRatio = m_size / m_glyphSize;

	m_glyphs = glyphVariable->m_glyphs;

	CGlyphText::AllocateResources();
}

/*
*/
void CGlyphText::AllocateResources()
{
	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(L"CGlyphFixed::CGlyphFixed");

	m_materialConstantBuffer = new CConstantBuffer();

	m_materialConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 104);

	m_materialConstantBuffer->CreateStaticResource();

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "GlyphFixed");

	m_shader->SetConstantBufferCount(2);
	m_shader->SetTextureCount(1);

	m_shaderHeap = m_shader->AllocateHeap();

	m_shaderHeap->BindResource(0, m_imageWvp->m_gpuBuffer.Get(), &m_imageWvp->m_srvDesc);
	m_shaderHeap->BindResource(1, m_materialConstantBuffer->m_gpuBuffer.Get(), &m_materialConstantBuffer->m_srvDesc);

	m_shaderHeap->BindResource(m_shader->m_t[0], m_glyphs->m_texture.Get(), &m_glyphs->m_srvDesc);

	m_glyphTextResource = new CGlyphTextResource[m_text->m_length]();

	m_index = 0;

	m_charPosition = m_position;

	for (int32_t i = 0; i < m_text->m_length; i++)
	{
		m_charIndex = m_text->m_text[i] - 0x20;

		if (m_charIndex >= 0)
		{
			m_glyphTextResource[m_index].m_position = m_charPosition;

			CGlyphText::UpdateResourceForText(&m_glyphTextResource[m_index], m_charIndex);

			CGlyphText::UpdatePosition(&m_glyphTextResource[m_index]);

			m_charPosition.m_p.x += (m_sizeRatio * m_glyphTextResource[m_index].m_width);
		}

		m_glyphTextResource[m_index].m_vertexBuffer = new CVertexBuffer();

		m_glyphTextResource[m_index].m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, CPipelineState::VertexType::E_VT_VERTEXT, 4, (void*)m_glyphTextResource[m_index].m_vertices);

		m_glyphTextResource[m_index].m_vertexBuffer->CreateDynamicResource();

		m_index++;
	}

	// vertex uv draw order
	// 0---1
	// | \ |
	// 3---2
	uint16_t index[6] = { 0, 1, 2, 0, 2, 3 };

	m_indexBuffer = new CIndexBuffer();

	m_indexBuffer->Constructor(m_graphicsAdapter, m_errorLog, 6, index);

	m_indexBuffer->CreateStaticResource();

	m_indexBuffer->RecordStatic();

	m_indexBuffer->UploadStaticResources();

	m_indexBuffer->ReleaseStaticCPUResource();
}

/*
*/
CGlyphText::~CGlyphText()
{
	SAFE_DELETE(m_indexBuffer);

	SAFE_DELETE_ARRAY(m_glyphTextResource);

	SAFE_DELETE(m_shaderHeap);
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_materialConstantBuffer);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);

	SAFE_DELETE(m_text);
}

/*
*/
void CGlyphText::Move(float x, float y)
{
	m_position.m_p.x += x;
	m_position.m_p.y += y;

	m_index = 0;

	m_charPosition = m_position;

	for (int32_t i = 0; i < m_text->m_length; i++)
	{
		m_charIndex = m_text->m_text[i] - 0x20;

		if (m_charIndex >= 0)
		{
			m_glyphTextResource[m_index].m_position = m_charPosition;

			CGlyphText::UpdateResourceForText(&m_glyphTextResource[m_index], m_charIndex);

			CGlyphText::UpdatePosition(&m_glyphTextResource[m_index]);

			m_charPosition.m_p.x += (m_sizeRatio * m_glyphTextResource[m_index].m_width);

			m_glyphTextResource[m_index].m_vertexBuffer->LoadDynamicBuffer();

			m_index++;
		}
	}
}

/*
*/
void CGlyphText::Record()
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

	m_commandList->IASetIndexBuffer(&m_indexBuffer->m_view);

	m_index = 0;

	for (int32_t i = 0; i < strlen(m_text->m_text); i++)
	{
		m_glyphTextResource[i].m_vertexBuffer->DrawIndexed(m_commandList, m_indexBuffer->m_count);
	}
}

/*
*/
void CGlyphText::SetColor(float* color)
{
	m_materialConstantBuffer->Reset();

	m_materialConstantBuffer->m_values->Append(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_materialConstantBuffer->m_values->Append(XMFLOAT4(color[0], color[1], color[2], color[3]));
}

/*
*/
void CGlyphText::SetText(const char* text)
{
	m_text->SetText(text);

	m_index = 0;

	m_charPosition = m_position;

	for (int32_t i = 0; i < m_text->m_length; i++)
	{
		m_charIndex = m_text->m_text[i] - 0x20;

		if (m_charIndex >= 0)
		{
			m_glyphTextResource[m_index].m_position = m_charPosition;

			CGlyphText::UpdateResourceForText(&m_glyphTextResource[m_index], m_charIndex);

			CGlyphText::UpdatePosition(&m_glyphTextResource[m_index]);

			m_charPosition.m_p.x += (m_sizeRatio * m_glyphTextResource[m_index].m_width);

			m_glyphTextResource[m_index].m_vertexBuffer->LoadDynamicBuffer();

			m_index++;
		}
	}
}

/*
*/
void CGlyphText::UpdatePosition(CGlyphTextResource* glyphTextResource)
{
	glyphTextResource->m_vertices[0].m_p.x = glyphTextResource->m_position.m_p.x;
	glyphTextResource->m_vertices[1].m_p.x = glyphTextResource->m_position.m_p.x + glyphTextResource->m_width * m_sizeRatio;
	glyphTextResource->m_vertices[2].m_p.x = glyphTextResource->m_position.m_p.x + glyphTextResource->m_width * m_sizeRatio;
	glyphTextResource->m_vertices[3].m_p.x = glyphTextResource->m_position.m_p.x;

	glyphTextResource->m_vertices[0].m_p.y = glyphTextResource->m_position.m_p.y;
	glyphTextResource->m_vertices[1].m_p.y = glyphTextResource->m_position.m_p.y;
	glyphTextResource->m_vertices[2].m_p.y = glyphTextResource->m_position.m_p.y + m_glyphSize * m_sizeRatio;
	glyphTextResource->m_vertices[3].m_p.y = glyphTextResource->m_position.m_p.y + m_glyphSize * m_sizeRatio;
}

/*
*/
void CGlyphText::UpdateResourceForText(CGlyphTextResource* glyphTextResource, int32_t glyphIndex)
{
	m_glyphVertice = (CGlyphVertices*)m_glyphVertices->GetElement(1, glyphIndex);

	glyphTextResource->m_width = m_glyphVertice->m_width;

	glyphTextResource->m_vertices[0].m_p.x = m_glyphVertice->m_vertices[0].m_p.x;
	glyphTextResource->m_vertices[0].m_p.y = m_glyphVertice->m_vertices[0].m_p.y;

	glyphTextResource->m_vertices[1].m_p.x = m_glyphVertice->m_vertices[1].m_p.x;
	glyphTextResource->m_vertices[1].m_p.y = m_glyphVertice->m_vertices[1].m_p.y;

	glyphTextResource->m_vertices[2].m_p.x = m_glyphVertice->m_vertices[2].m_p.x;
	glyphTextResource->m_vertices[2].m_p.y = m_glyphVertice->m_vertices[2].m_p.y;

	glyphTextResource->m_vertices[3].m_p.x = m_glyphVertice->m_vertices[3].m_p.x;
	glyphTextResource->m_vertices[3].m_p.y = m_glyphVertice->m_vertices[3].m_p.y;

	glyphTextResource->m_vertices[0].m_uv.x = m_glyphVertice->m_vertices[0].m_uv.x;
	glyphTextResource->m_vertices[0].m_uv.y = m_glyphVertice->m_vertices[0].m_uv.y;

	glyphTextResource->m_vertices[1].m_uv.x = m_glyphVertice->m_vertices[1].m_uv.x;
	glyphTextResource->m_vertices[1].m_uv.y = m_glyphVertice->m_vertices[1].m_uv.y;

	glyphTextResource->m_vertices[2].m_uv.x = m_glyphVertice->m_vertices[2].m_uv.x;
	glyphTextResource->m_vertices[2].m_uv.y = m_glyphVertice->m_vertices[2].m_uv.y;

	glyphTextResource->m_vertices[3].m_uv.x = m_glyphVertice->m_vertices[3].m_uv.x;
	glyphTextResource->m_vertices[3].m_uv.y = m_glyphVertice->m_vertices[3].m_uv.y;
}