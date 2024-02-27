#include "CShadowMap.h"

/*
*/
CShadowMap::CShadowMap()
{
	memset(this, 0x00, sizeof(CShadowMap));
}

/*
*/
CShadowMap::CShadowMap(CErrorLog* errorLog, CVideoDevice* videoDevice, CShaderManager* shaderManager, CRootSignature* rootSignature, CShaderHeap* shaderHeap, int slot)
{
	memset(this, 0x00, sizeof(CShadowMap));

	m_errorLog = errorLog;

	m_videoDevice = videoDevice;


	m_depthBuffer = new CDepthBuffer(errorLog, m_videoDevice->m_device, m_videoDevice->m_width, m_videoDevice->m_height, true);

	m_shadowMap = new CTexture(m_videoDevice, errorLog);


	m_pipelineState = new CPipelineState(m_videoDevice, errorLog, CVertex::E_VT_VERTEXNT);

	m_vertexShader = shaderManager->Create("vertexTerrain.hlsl", "VSMain", "vs_5_1");
	m_pixelShader = shaderManager->Create("pixelTerrain.hlsl", "PSMain", "ps_5_1");

	m_pipelineState->CreateShadowMap(D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_vertexShader, m_pixelShader, rootSignature->m_signature);

	shaderHeap->SlotResource(slot, m_shadowMap, DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	m_firstRecord = true;
}

/*
*/
CShadowMap::~CShadowMap()
{
	delete m_depthBuffer;
	delete m_shadowMap;
	delete m_pipelineState;
}

/*
*/
void CShadowMap::PreRecord(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	/**/
	if (m_firstRecord)
	{
		m_firstRecord = false;
	}
	else
	{
		m_depthBuffer->m_barrier.Transition.pResource = m_depthBuffer->m_resource.Get();
		m_depthBuffer->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
		m_depthBuffer->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;

		commandList->ResourceBarrier(1, &m_depthBuffer->m_barrier);
	}


	// Clear the back buffer and depth buffer.
	commandList->ClearDepthStencilView(m_depthBuffer->m_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set null render target because we are only going to draw to
	// depth buffer.  Setting a null render target will disable color writes.
	// Note the active PSO also must specify a render target count of 0.
	commandList->OMSetRenderTargets(0, nullptr, false, &m_depthBuffer->m_handle);

	/*
	// Bind the pass constant buffer for the shadow map pass.
	auto passCB = mCurrFrameResource->PassCB->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
	commandList->SetGraphicsRootConstantBufferView(1, passCBAddress);

	commandList->SetPipelineState(mPSOs["shadow_opaque"].Get());
	*/

	commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);
}

void CShadowMap::PostRecord(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// Change back to GENERIC_READ so we can read the texture in a shader.
	/**/
	m_depthBuffer->m_barrier.Transition.pResource = m_depthBuffer->m_resource.Get();
	m_depthBuffer->m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_depthBuffer->m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;

	m_depthBuffer->SetPreDrawBarrier();

	commandList->ResourceBarrier(1, &m_depthBuffer->m_barrier);
}