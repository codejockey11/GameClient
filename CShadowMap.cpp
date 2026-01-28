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

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(L"CShadowMap::CShadowMap");

	m_viewport.m_viewport = { 0.0f, 0.0f, 1024, 1024, 0.0f, 1.0f };

	m_viewport.m_scissorRect = { 0, 0, 1024, 1024 };

	m_depthBuffer = new CDepthBuffer(m_errorLog, m_graphicsAdapter->m_device, 2, 1024, 1024);

	m_depthBuffer->CreateShadowMap();

	m_vs = m_shaderBinaryManager->Create(m_local->m_installPath->m_text, "shadows.vs", CShaderBinaryManager::BinaryType::VERTEX);
	m_ps = m_shaderBinaryManager->Create(m_local->m_installPath->m_text, "shadows.ps", CShaderBinaryManager::BinaryType::PIXEL);

	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, 1, 1, CPipelineState::VertexType::E_VT_VERTEX, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->SetPixelShader(m_ps);
	m_pipelineState->SetVertexShader(m_vs);

	m_pipelineState->CreateShadowMap(D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_constantBuffer = new CConstantBuffer();

	m_constantBuffer->Constructor(m_graphicsAdapter, m_errorLog, sizeof(XMFLOAT4X4) * 3);

	m_constantBuffer->CreateStaticResource();

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Shadows");

	m_shader->SetConstantBufferCount(1);
	m_shader->SetTextureCount(1);

	m_shaderHeap = m_shader->AllocateHeap();

	m_light = new CLight();

	m_lightPosition = CVec3f(0.0f, 0.0f, 250.0f);

	CVec3f target = CVec3f(0.0f, 0.0f, -512.0f);

	m_light->Constructor(&m_lightPosition, &target, 32768.0f);

	m_light->m_vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_light->InitForShadow();

	m_constantBuffer->m_values->Append(m_light->m_world);
	m_constantBuffer->m_values->Append(m_light->m_view);
	m_constantBuffer->m_values->Append(m_light->m_proj);

	m_constantBuffer->RecordStatic();

	m_constantBuffer->UploadStaticResources();

	m_shaderHeap->BindResource(0, m_constantBuffer->m_gpuBuffer.Get(), &m_constantBuffer->m_srvDesc);
}

/*
*/
CShadowMap::~CShadowMap()
{
	SAFE_DELETE(m_light);
	SAFE_DELETE(m_shaderHeap);
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_constantBuffer);
	SAFE_DELETE(m_pipelineState);

	SAFE_DELETE(m_depthBuffer);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
}

/*
*/
ID3D12Resource* CShadowMap::GetResource()
{
	return m_depthBuffer->m_resource.Get();
}

/*
*/
D3D12_SHADER_RESOURCE_VIEW_DESC* CShadowMap::GetSRV()
{
	return &m_depthBuffer->m_srvDesc;
}

/*
*/
void CShadowMap::PostRecord()
{
	m_depthBuffer->m_barrier.Transition.pResource = m_depthBuffer->m_resource.Get();
	m_depthBuffer->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_depthBuffer->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;

	m_commandList->ResourceBarrier(1, &m_depthBuffer->m_barrier);

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

	m_commandList->OMSetRenderTargets(0, nullptr, false, &m_depthBuffer->m_handle);

	m_depthBuffer->m_barrier.Transition.pResource = m_depthBuffer->m_resource.Get();
	m_depthBuffer->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
	m_depthBuffer->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_depthBuffer->m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &m_depthBuffer->m_barrier);

	m_commandList->ClearDepthStencilView(m_depthBuffer->m_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());

	m_shaderHeap->BindResource(m_shader->m_t[0], m_depthBuffer->m_resource.Get(), &m_depthBuffer->m_srvDesc);

	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap->GetBaseDescriptor());
}