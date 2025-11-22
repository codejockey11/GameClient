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

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(L"CShadowMap::CShadowMap");

	m_commandList->Close();

	m_viewport.m_viewport = { 0.0f, 0.0f, 2048.0f, 2048.0f, 0.0f, 1.0f };

	m_viewport.m_scissorRect = { 0, 0, 2048, 2048 };

	for (int32_t i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_depthBuffers[i] = new CDepthBuffer(m_errorLog, m_graphicsAdapter->m_device, 2, 2048, 2048);

		m_depthBuffers[i]->CreateShadowMap();
	}

	m_vs = m_shaderBinaryManager->Create("shadows.vs", CShaderBinaryManager::BinaryType::VERTEX);
	m_ps = m_shaderBinaryManager->Create("shadows.ps", CShaderBinaryManager::BinaryType::PIXEL);
	
	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, VertexType::E_VT_VERTEX, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->AllocateSignature(1, 1);

	m_pipelineState->SetPixelShader(m_ps);
	m_pipelineState->SetVertexShader(m_vs);

	m_pipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_constantBuffer = new CConstantBuffer();

	m_constantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, sizeof(XMFLOAT4X4) * 3);

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Shadows");

	m_shader->SetConstantBufferCount(1);
	m_shader->SetTextureCount(1);

	m_shader->AllocateHeap();

	m_light = new CLight();

	m_lightPosition = CVec3f(0.0f, 0.0f, 256.0f);

	CVec3f target = CVec3f(0.0f, 0.0f, -200.0f);

	m_light->Constructor(&m_lightPosition, &target, 32768.0f);

	m_light->InitForShadow();

	m_constantBuffer->m_values->Append(m_light->m_world);
	m_constantBuffer->m_values->Append(m_light->m_view);
	m_constantBuffer->m_values->Append(m_light->m_proj);
}

/*
*/
CShadowMap::~CShadowMap()
{
	SAFE_DELETE(m_light);
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_constantBuffer);
	SAFE_DELETE(m_pipelineState);

	for (int32_t i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		SAFE_DELETE(m_depthBuffers[i]);
	}

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
}

/*
*/
ID3D12Resource* CShadowMap::GetResource()
{
	return m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get();
}

/*
*/
D3D12_SHADER_RESOURCE_VIEW_DESC* CShadowMap::GetSRV()
{
	return &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_srvDesc;
}

/*
*/
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

/*
*/
void CShadowMap::PreRecord()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_viewport.m_viewport);

	m_commandList->RSSetScissorRects(1, &m_viewport.m_scissorRect);

	m_commandList->OMSetRenderTargets(0, nullptr, false, &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle);

	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.pResource = m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get();
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_barrier);

	m_commandList->ClearDepthStencilView(m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());

	m_constantBuffer->UpdateBuffer();

	m_shader->BindResource(m_shader->m_t[0], m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_resource.Get(), &m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_srvDesc);

	m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());
}