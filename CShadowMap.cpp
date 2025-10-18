#include "CShadowMap.h"

/*
*/
CShadowMap::CShadowMap()
{
	memset(this, 0x00, sizeof(CShadowMap));
}

/*
*/
CShadowMap::CShadowMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinaryManager* shaderBinaryManager)
{
	memset(this, 0x00, sizeof(CShadowMap));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_shaderBinaryManager = shaderBinaryManager;

	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(L"CShadowMap::CShadowMap");

	m_commandList->Close();

	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_depthBuffers[i] = new CDepthBuffer(m_errorLog, m_graphicsAdapter->m_device, m_graphicsAdapter->m_width, m_graphicsAdapter->m_height);
		
		m_depthBuffers[i]->CreateShadowMap();
	}

	m_texture = new CTexture(m_graphicsAdapter, m_errorLog, m_local, nullptr, "Shadows");

	m_texture->CreateForShadowMap();

	m_vs = m_shaderBinaryManager->Create("Shadows.vs", CShaderBinaryManager::BinaryType::VERTEX);
	m_ps = m_shaderBinaryManager->Create("Shadows.ps", CShaderBinaryManager::BinaryType::PIXEL);

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Shadows");

	m_shader->AllocateConstantBuffers(1);

	m_shader->CreateConstantBuffer(m_shader->m_b[0], (sizeof(XMFLOAT4X4) * 3), m_commandList);

	m_shader->SetTextureCount(1);

	m_shader->AllocateDescriptorRange(2);

	m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
	m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

	m_shader->AllocateRootParameter();

	m_shader->AllocatePipelineStateForShadowMap(VertexType::E_VT_VERTEXNT, m_vs, m_ps);

	m_shader->BindConstantBuffers();

	m_shader->BindResource(m_shader->m_t[0], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

	CVec3f position = CVec3f(236.0f, 80.0f, 0.0f);
	CVec3f target = CVec3f(0.0f, -256.0f, 0.0f);

	m_light.Constructor(&position, &target, 0.0f);

	m_light.InitForShadow();

	m_shader->m_constantBuffer[m_shader->m_b[0]].Reset();

	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_light.m_world);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_light.m_view);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_light.m_proj);
}

/*
*/
CShadowMap::~CShadowMap()
{
	delete m_texture;

	m_commandList.Reset();

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		delete m_depthBuffers[i];

		m_commandAllocators[i].Reset();
	}

	delete m_shader;
}

/*
*/
void CShadowMap::PreRecord()
{
	m_commandAllocators[m_graphicsAdapter->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_graphicsAdapter->m_backbufferIndex].Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_graphicsAdapter->m_swapChainViewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &m_graphicsAdapter->m_swapChainViewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(0, nullptr, false, &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle);

	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.pResource = m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get();
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier);

	m_commandList->ClearDepthStencilView(m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_shader->UpdateConstantBuffers();

	m_commandList->SetGraphicsRootSignature(m_shader->m_rootSignature->m_signature.Get());

	m_commandList->SetDescriptorHeaps(1, m_shader->m_heap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());
	
	m_commandList->SetPipelineState(m_shader->m_pipelineState->m_pipelineState.Get());
}

void CShadowMap::PostRecord()
{
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.pResource = m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get();
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;

	m_commandList->ResourceBarrier(1, &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier);

	m_commandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_commandList.GetAddressOf());

	m_graphicsAdapter->WaitForGPU();
}