#include "COverhead.h"

/*
*/
COverhead::COverhead()
{
	memset(this, 0x00, sizeof(COverhead));
}

/*
*/
COverhead::COverhead(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager, CVec2f position, CVec2f size)
{
	memset(this, 0x00, sizeof(COverhead));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_shaderBinaryManager = shaderBinaryManager;

	m_position = position;

	m_size = size;

	float xScale = (m_graphicsAdapter->m_swapChainViewport->m_viewport.Width > 0) ?
		2.0f / m_graphicsAdapter->m_swapChainViewport->m_viewport.Width : 0.0f;
	
	float yScale = (m_graphicsAdapter->m_swapChainViewport->m_viewport.Height > 0) ?
		2.0f / m_graphicsAdapter->m_swapChainViewport->m_viewport.Height : 0.0f;

	XMFLOAT4X4 wvpMat
	(
		xScale, 0, 0, 0,
		0, -yScale, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	);

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));

		m_renderTargets[i] = new CRenderTarget(m_errorLog, m_graphicsAdapter->m_device);

		m_renderTargets[i]->AdditionalTarget(m_resources[i], (UINT64)size.m_p.x, (UINT)size.m_p.y, m_graphicsAdapter->m_backBufferFormat);

		m_depthBuffers[i] = new CDepthBuffer(m_errorLog, m_graphicsAdapter->m_device, (UINT)size.m_p.x, (UINT)size.m_p.y);
		
		m_depthBuffers[i]->Create();
	}

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(L"COverhead::COverhead");

	m_commandList->Close();

	m_texture = new CTexture(m_graphicsAdapter, m_errorLog, nullptr, nullptr, "Overhead");

	m_texture->SetCommandList(m_commandList);

	m_texture->CreateDynamic((UINT)m_size.m_p.x, (UINT)m_size.m_p.y);

	m_vs = m_shaderBinaryManager->Get("Image.vs");
	m_ps = m_shaderBinaryManager->Get("Image.ps");

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Overhead");

	m_shader->AllocateConstantBuffers(1);

	m_shader->CreateConstantBuffer(m_shader->m_b[0], sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4), m_commandList);

	m_shader->SetTextureCount(1);

	m_shader->AllocateDescriptorRange(2);

	m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
	m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

	m_shader->AllocateRootParameter();

	m_shader->AllocatePipelineState(false, VertexType::E_VT_VERTEXT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

	m_shader->BindConstantBuffers();

	m_shader->m_constantBuffer[m_shader->m_b[0]].Reset();

	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(wvpMat);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(1.0f);

	m_shader->BindResource(m_shader->m_t[0], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

	m_vertices[0].m_p.x = 0.0f;
	m_vertices[0].m_p.y = 0.0f;

	m_vertices[0].m_uv.x = 0.0f;
	m_vertices[0].m_uv.y = 0.0f;

	m_vertices[1].m_p.x = m_size.m_p.x;
	m_vertices[1].m_p.y = 0.0f;

	m_vertices[1].m_uv.x = 1.0f;
	m_vertices[1].m_uv.y = 0.0f;

	m_vertices[2].m_p.x = m_size.m_p.x;
	m_vertices[2].m_p.y = m_size.m_p.y;

	m_vertices[2].m_uv.x = 1.0f;
	m_vertices[2].m_uv.y = 1.0f;

	m_vertices[3].m_p.x = 0.0f;
	m_vertices[3].m_p.y = m_size.m_p.y;

	m_vertices[3].m_uv.x = 0.0f;
	m_vertices[3].m_uv.y = 1.0f;

	m_vertexBuffer = new CVertexBuffer();

	m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, VertexType::E_VT_VERTEXT, 4, (void*)m_vertices);

	DWORD index[6] = {0, 1, 2, 0, 2, 3};
	/*
	index[0] = 0;
	index[1] = 1;
	index[2] = 2;

	index[3] = 0;
	index[4] = 2;
	index[5] = 3;
	*/
	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, m_errorLog, 6, index);

	m_viewport = new CViewport(0.0f, 0.0f, m_size.m_p.x, m_size.m_p.y);

	m_camera = new CCamera();

	m_camera->Constructor(m_size.m_p.x, m_size.m_p.y, (CVec3f*)&m_position, 45.0f, 1.0f, 50000.0f, (m_size.m_p.x / m_size.m_p.y));

	m_camera->UpdateRotation(90.0f, 0.0F, 0.0f);
	m_camera->UpdateView();

	COverhead::UpdatePosition();
}

/*
*/
COverhead::~COverhead()
{
	delete m_camera;
	delete m_viewport;
	delete m_indexBuffer;
	delete m_vertexBuffer;
	delete m_texture;

	m_commandList.Reset();

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		delete m_depthBuffers[i];

		m_resources[i].Reset();

		delete m_renderTargets[i];

		m_commandAllocators[i].Reset();
	}

	delete m_shader;
}

/*
*/
void COverhead::ClearTargets()
{
	m_graphicsAdapter->m_commandList->ClearRenderTargetView(
		m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		m_graphicsAdapter->m_color->LovelyPurple->GetFloatArray(),
		0,
		nullptr);

	m_graphicsAdapter->m_commandList->ClearDepthStencilView(
		m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr);
}

/*
*/
void COverhead::Record()
{
	m_commandAllocators[m_graphicsAdapter->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_graphicsAdapter->m_backbufferIndex].Get(), nullptr);

	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.pResource = m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get();
	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE;
	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier);

	m_commandList->CopyResource(m_texture->m_texture.Get(), m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get());

	m_texture->m_barrier.Transition.pResource = m_texture->m_texture.Get();
	m_texture->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
	m_texture->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_commandList->ResourceBarrier(1, &m_texture->m_barrier);

	m_commandList->RSSetViewports(1, &m_graphicsAdapter->m_swapChainViewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &m_graphicsAdapter->m_swapChainViewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&m_graphicsAdapter->m_swapChainRenderTargets[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		false,
		&m_graphicsAdapter->m_swapChainDepthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_shader->UpdateConstantBuffers();

	m_commandList->SetGraphicsRootSignature(m_shader->m_rootSignature->m_signature.Get());

	m_commandList->SetDescriptorHeaps(1, m_shader->m_heap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->SetPipelineState(m_shader->m_pipelineState->m_pipelineState.Get());

	m_indexBuffer->SetCommandList(m_commandList);
	m_indexBuffer->Draw();

	m_vertexBuffer->SetCommandList(m_commandList);
	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);

	m_texture->m_barrier.Transition.pResource = m_texture->m_texture.Get();
	m_texture->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_texture->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;

	m_commandList->ResourceBarrier(1, &m_texture->m_barrier);

	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.pResource = m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get();
	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE;
	m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;

	m_commandList->ResourceBarrier(1, &m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_barrier);
}

/*
*/
void COverhead::SetCameraPosition(XMFLOAT3 position)
{
	m_camera->m_position = position;

	m_camera->m_position.y = position.y + 32.0f;
}

/*
*/
void COverhead::UpdatePosition()
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