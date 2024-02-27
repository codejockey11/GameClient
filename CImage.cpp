#include "CImage.h"

/*
*/
CImage::CImage()
{
	memset(this, 0x00, sizeof(CImage));
}

/*
*/
CImage::CImage(CVideoDevice* videoDevice, CErrorLog* errorLog, CTexture* texture, CShaderManager* shaderManager, CVertex2 size, CVertex2 position)
{
	memset(this, 0x00, sizeof(CImage));

	m_videoDevice = videoDevice;

	m_texture = texture;

	m_size = size;

	m_position = position;

	// Need to make individual command allocators and command lists to accomodate threading
	for (UINT i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i] = m_videoDevice->CreateCommandAllocator();
	}

	m_commandList = m_videoDevice->CreateCommandList(m_commandAllocators[0]);

	m_commandList->SetName(m_texture->m_name->GetWText());

	m_commandList->Close();


	m_wvp = new CFloat4x4Buffer(m_videoDevice, errorLog, m_commandList);

	m_floats = new CFloatBuffer(m_videoDevice, errorLog, m_commandList);

	m_ints = new CIntBuffer(m_videoDevice, errorLog, m_commandList);


	float xScale = (m_videoDevice->m_swapChainViewport->m_viewport.Width > 0) ?
		2.0f / m_videoDevice->m_swapChainViewport->m_viewport.Width : 0.0f;
	float yScale = (m_videoDevice->m_swapChainViewport->m_viewport.Height > 0) ?
		2.0f / m_videoDevice->m_swapChainViewport->m_viewport.Height : 0.0f;

	XMMATRIX wvpMat
	(
		xScale, 0, 0, 0,
		0, -yScale, 0, 0,
		0, 0, 1, 0,
		-1, 1, 0, 1
	);

	XMStoreFloat4x4(&m_wvp->m_values[0], wvpMat);

	
	m_shaderHeap = new CShaderHeap(m_videoDevice, errorLog, 4);

	// these resources must be mapped to the pixel shader buffers
	m_shaderHeap->SlotResource(0, m_wvp);
	m_shaderHeap->SlotResource(1, m_floats);
	m_shaderHeap->SlotResource(2, m_ints);
	m_shaderHeap->SlotResource(3, m_texture, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);


	m_range = new CDescriptorRange(2);

	// the number three represents the number of constant buffers
	m_range->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3);

	// the number one represents the number of texture buffers
	m_range->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);

	// the root signature defines the number of samplers
	m_rootSignature = new CRootSignature(m_videoDevice, errorLog,
		D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
		2,
		m_range);


	m_vertexShader = shaderManager->Create("vertexImage.hlsl", "VSMain", "vs_5_1");
	m_pixelShader = shaderManager->Create("pixelImage.hlsl", "PSMain", "ps_5_1");


	m_pipelineState = new CPipelineState(m_videoDevice, errorLog, CVertex::E_VT_VERTEXT);

	m_pipelineState->Create(false, true, false,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		m_vertexShader,	m_pixelShader, nullptr, nullptr, nullptr,
		m_rootSignature->m_signature);


	// vertex uv draw order
	// 0---1
	// | \ |
	// 3---2

	m_vertices[0].p.x = 0.0f;
	m_vertices[0].p.y = 0.0f;

	m_vertices[0].uv.x = 0.0f;
	m_vertices[0].uv.y = 0.0f;

	m_vertices[1].p.x = size.p.x;
	m_vertices[1].p.y = 0.0f;

	m_vertices[1].uv.x = 1.0f;
	m_vertices[1].uv.y = 0.0f;

	m_vertices[2].p.x = size.p.x;
	m_vertices[2].p.y = size.p.y;

	m_vertices[2].uv.x = 1.0f;
	m_vertices[2].uv.y = 1.0f;

	m_vertices[3].p.x = 0.0f;
	m_vertices[3].p.y = size.p.y;

	m_vertices[3].uv.x = 0.0f;
	m_vertices[3].uv.y = 1.0f;

	m_vertexBuffer = new CVertexBuffer(m_videoDevice, errorLog, m_commandList, CVertex::E_VT_VERTEXT, 4, (void*)m_vertices);

	
	DWORD index[6] = {};

	index[0] = 0;
	index[1] = 1;
	index[2] = 2;

	index[3] = 0;
	index[4] = 2;
	index[5] = 3;

	m_indexBuffer = new CIndexBuffer(m_videoDevice, errorLog, m_commandList, 6, index);

	CVertex v1(m_vertices[0].p.x, m_vertices[0].p.y, 0.0f);
	CVertex v2(m_vertices[2].p.x, m_vertices[2].p.y, 0.0f);

	m_box = new CBoundBox(&v1, &v2);

	CImage::UpdatePosition();
}

/*
*/
CImage::~CImage()
{
	delete m_box;
	delete m_indexBuffer;
	delete m_vertexBuffer;
	delete m_pipelineState;
	delete m_rootSignature;
	delete m_range;
	delete m_shaderHeap;
	delete m_ints;
	delete m_floats;
	delete m_wvp;

	m_commandList.Reset();

	for (int i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}
}

/*
*/
void CImage::Draw()
{
	m_commandAllocators[m_videoDevice->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_videoDevice->m_backbufferIndex].Get(), nullptr);


	m_commandList->RSSetViewports(1, &m_videoDevice->m_swapChainViewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &m_videoDevice->m_swapChainViewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&m_videoDevice->m_swapChainRenderTargets[m_videoDevice->m_backbufferIndex]->m_handle,
		false,
		nullptr);


	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetGraphicsRootSignature(m_rootSignature->m_signature.Get());

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());


	m_wvp->UpdateConstantBuffer();

	m_floats->UpdateConstantBuffer();

	m_ints->UpdateConstantBuffer();


	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap->m_heap->GetGPUDescriptorHandleForHeapStart());


	m_indexBuffer->Draw();

	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);
}

/*
*/
void CImage::UpdatePosition()
{
	m_vertices[0].p.x = m_position.p.x;
	m_vertices[1].p.x = m_position.p.x + m_size.p.x;
	m_vertices[2].p.x = m_position.p.x + m_size.p.x;
	m_vertices[3].p.x = m_position.p.x;

	m_vertices[0].p.y = m_position.p.y;
	m_vertices[1].p.y = m_position.p.y;
	m_vertices[2].p.y = m_position.p.y + m_size.p.y;
	m_vertices[3].p.y = m_position.p.y + m_size.p.y;

	if ((m_vertices[1].p.x < 0.0f) || (m_vertices[3].p.y < 0.0f) ||
		(m_vertices[0].p.x > m_videoDevice->m_width) ||
		(m_vertices[1].p.y > m_videoDevice->m_height))
	{
		return;
	}

	CVertex v1(m_vertices[0].p.x, m_vertices[0].p.y, 0.0f);
	CVertex v2(m_vertices[2].p.x, m_vertices[2].p.y, 0.0f);

	m_box->Update(&v1, &v2);

	m_vertexBuffer->LoadBuffer((void*)m_vertices);
}