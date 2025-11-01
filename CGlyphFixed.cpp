#include "CGlyphFixed.h"

/*
*/
CGlyphFixed::CGlyphFixed()
{
	memset(this, 0x00, sizeof(CGlyphFixed));
}

/*
*/
CGlyphFixed::CGlyphFixed(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager, CTexture* glyphs, int glyphWidth, int size)
{
	memset(this, 0x00, sizeof(CGlyphFixed));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_shaderBinaryManager = shaderBinaryManager;

	m_glyphs = glyphs;

	m_glyphWidth = (float)glyphWidth;
	
	m_glyphHeight = (float)m_glyphs->m_height;
	
	m_glyphU = m_glyphWidth / (float)m_glyphs->m_width;
	m_glyphV = 1.0f;

	m_size = (float)size;

	m_sizeRatio = m_size / m_glyphHeight;

	float xScale = (m_graphicsAdapter->m_swapChainViewport->m_viewport.Width > 0) ? 2.0f / m_graphicsAdapter->m_swapChainViewport->m_viewport.Width : 0.0f;
	float yScale = (m_graphicsAdapter->m_swapChainViewport->m_viewport.Height > 0) ? 2.0f / m_graphicsAdapter->m_swapChainViewport->m_viewport.Height : 0.0f;

	m_wvpMat = XMFLOAT4X4
	(
		xScale, 0, 0, 0,
		0, -yScale, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	);

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(L"CGlyphFixed::CGlyphFixed");

	m_commandList->Close();

	m_vs = m_shaderBinaryManager->Get("image.vs");
	m_ps = m_shaderBinaryManager->Get("image.ps");

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "GlyphFixed");

	m_shader->AllocateConstantBuffers(1);

	m_shader->CreateConstantBuffer(0, sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4), m_commandList);

	m_shader->SetTextureCount(1);

	m_shader->AllocateDescriptorRange(2);

	m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1);
	m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);

	m_shader->AllocateRootParameter();

	m_shader->AllocatePipelineState(false, VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

	m_shader->BindConstantBuffers();

	m_shader->m_constantBuffers[0].Reset();

	m_shader->m_constantBuffers[0].m_values->Append(m_wvpMat);
	m_shader->m_constantBuffers[0].m_values->Append(1.0f);

	m_shader->BindResource(m_shader->m_t[0], m_glyphs->m_texture.Get(), &m_glyphs->m_srvDesc);

	// vertex uv draw order
	// 0---1
	// | \ |
	// 3---2

	m_glyphVertices = new CHeapArray(true, sizeof(CGlyphVertices), 1, 94);

	for (int i = 0; i < 94; i++)
	{
		m_glyphVertice = (CGlyphVertices*)m_glyphVertices->GetElement(1, i);

		m_glyphVertice->m_width = m_glyphWidth;

		m_glyphVertice->m_vertices[0].m_p.x = 0.0f;
		m_glyphVertice->m_vertices[0].m_p.y = 0.0f;

		m_glyphVertice->m_vertices[1].m_p.x = m_glyphVertice->m_width * m_sizeRatio;
		m_glyphVertice->m_vertices[1].m_p.y = 0.0f;

		m_glyphVertice->m_vertices[2].m_p.x = m_glyphVertice->m_width * m_sizeRatio;
		m_glyphVertice->m_vertices[2].m_p.y = m_glyphHeight * m_sizeRatio;

		m_glyphVertice->m_vertices[3].m_p.x = 0.0f;
		m_glyphVertice->m_vertices[3].m_p.y = m_glyphHeight * m_sizeRatio;

		m_glyphVertice->m_vertices[0].m_uv.x = (float)i * m_glyphU;
		m_glyphVertice->m_vertices[0].m_uv.y = 0.0f;

		m_glyphVertice->m_vertices[1].m_uv.x = ((float)i + 1.0f) * m_glyphU;
		m_glyphVertice->m_vertices[1].m_uv.y = 0.0f;

		m_glyphVertice->m_vertices[2].m_uv.x = ((float)i + 1.0f) * m_glyphU;
		m_glyphVertice->m_vertices[2].m_uv.y = m_glyphV;

		m_glyphVertice->m_vertices[3].m_uv.x = (float)i * m_glyphU;
		m_glyphVertice->m_vertices[3].m_uv.y = m_glyphV;
	}
}

/*
*/
CGlyphFixed::~CGlyphFixed()
{
	delete m_glyphVertices;

	m_commandList.Reset();

	delete m_commandAllocator;

	delete m_shader;
}

/*
*/
void CGlyphFixed::Record()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	m_commandList->RSSetViewports(1, m_graphicsAdapter->GetViewport());

	m_commandList->RSSetScissorRects(1, m_graphicsAdapter->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, m_graphicsAdapter->GetRenderTarget(), false, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetGraphicsRootSignature(m_shader->GetRootSignature());

	m_shader->m_constantBuffers[0].UpdateBuffer();

	m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

	m_commandList->SetPipelineState(m_shader->GetPipelineState());
}