#include "CGlyphText.h"

/*
*/
CGlyphText::CGlyphText()
{
	memset(this, 0x00, sizeof(CGlyphText));
}

/*
*/
CGlyphText::CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CGlyphFixed* glyphFixed, int32_t bundle,
	const char* text, CVec2f position, float size)
{
	memset(this, 0x00, sizeof(CGlyphText));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_shaderBinaryManager = shaderBinaryManager;

	m_bundle = bundle;

	m_glyphVertices = glyphFixed->m_glyphVertices;

	m_text = new CString(text);

	m_position = position;

	m_size = size;

	m_glyphSize = glyphFixed->m_glyphHeight;

	m_sizeRatio = m_size / m_glyphSize;

	m_glyphs = glyphFixed->m_glyphs;

	CGlyphText::AllocateResources();
}

/*
*/
CGlyphText::CGlyphText(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinaryManager* shaderBinaryManager, CGlyphVariable* glyphVariable, int32_t bundle,
	const char* text, CVec2f position, float size)
{
	memset(this, 0x00, sizeof(CGlyphText));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_shaderBinaryManager = shaderBinaryManager;

	m_bundle = bundle;

	m_glyphVertices = glyphVariable->m_glyphVertices;

	m_text = new CString(text);

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
	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(L"CGlyphFixed::CGlyphFixed");

	m_commandList->Close();

	m_vs = m_shaderBinaryManager->Get("image.vs");
	m_ps = m_shaderBinaryManager->Get("image.ps");

	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->AllocateSignature(1, 1);

	m_pipelineState->SetPixelShader(m_ps);
	m_pipelineState->SetVertexShader(m_vs);

	m_pipelineState->Create(false, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_constantBuffer = new CConstantBuffer();

	m_constantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList,
		sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4));

	m_constantBuffer->m_values->Append(m_graphicsAdapter->m_wvpMat);
	m_constantBuffer->m_values->Append(1.0f);

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "GlyphFixed");

	m_shader->SetConstantBufferCount(1);
	m_shader->SetTextureCount(1);

	m_shader->AllocateHeap();

	m_shader->BindResource(0, m_constantBuffer->m_buffer.Get(), &m_constantBuffer->m_srvDesc);

	m_shader->BindResource(m_shader->m_t[0], m_glyphs->m_texture.Get(), &m_glyphs->m_srvDesc);

	m_glyphTextResource = new CGlyphTextResource[m_text->m_length]();

	m_index = 0;

	for (int32_t i = 0; i < m_text->m_length; i++)
	{
		m_charIndex = m_text->m_text[i] - 0x21;

		if (m_charIndex >= 0)
		{
			CGlyphText::UpdateResourceForText(&m_glyphTextResource[m_index], m_charIndex);
		}

		m_glyphTextResource[m_index].m_vertexBuffer = new CVertexBuffer();

		m_glyphTextResource[m_index].m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, VertexType::E_VT_VERTEXT, 4, (void*)m_glyphTextResource[m_index].m_vertices);

		m_glyphTextResource[m_index].m_vertexBuffer->SetCommandList(m_commandList);

		m_index++;
	}

	uint16_t index[6] = { 0, 1, 2, 0, 2, 3 };

	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, m_errorLog, 6, index);
}

/*
*/
CGlyphText::~CGlyphText()
{
	SAFE_DELETE(m_indexBuffer);
	
	SAFE_DELETE_ARRAY(m_glyphTextResource);

	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_constantBuffer);
	SAFE_DELETE(m_pipelineState);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);

	SAFE_DELETE(m_text);
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

	m_constantBuffer->UpdateBuffer();

	m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

	m_indexBuffer->SetCommandList(m_commandList);

	m_indexBuffer->Record();

	m_charPosition = m_position;

	m_index = 0;

	for (int32_t i = 0; i < strlen(m_text->m_text); i++)
	{
		m_charIndex = m_text->m_text[i] - 0x21;

		if (m_charIndex >= 0)
		{
			m_glyphTextResource[m_index].m_position = m_charPosition;

			CGlyphText::UpdateResourceForText(&m_glyphTextResource[m_index], m_charIndex);

			CGlyphText::UpdatePosition(&m_glyphTextResource[m_index]);

			m_glyphTextResource[m_index].m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);

			m_charPosition.m_p.x += (m_sizeRatio * m_glyphTextResource[m_index].m_width);
		}
		else
		{
			m_charPosition.m_p.x += (m_sizeRatio * 8.0f);
		}

		m_index++;
	}

	m_graphicsAdapter->BundleCommandList(m_bundle, m_commandList);
}

/*
*/
void CGlyphText::SetText(const char* text)
{
	if (m_text)
	{
		SAFE_DELETE(m_text);
	}

	m_text = new CString(text);
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

	glyphTextResource->m_vertexBuffer->LoadBuffer((void*)glyphTextResource->m_vertices);
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