#include "CTerrain.h"

/*
*/
CTerrain::CTerrain()
{
	memset(this, 0x00, sizeof(CTerrain));
}

/*
*/
CTerrain::CTerrain(CVideoDevice* videoDevice, CErrorLog* errorLog, CTextureManager* textureManager, CShaderManager* shaderManager, const char* filename)
{
	memset(this, 0x00, sizeof(CTerrain));

	m_videoDevice = videoDevice;

	m_name = new CString(filename);


	for (UINT i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i] = m_videoDevice->CreateCommandAllocator();
	}

	m_commandList = m_videoDevice->CreateCommandList(m_commandAllocators[0]);

	m_commandList->SetName(m_name->GetWText());

	m_commandList->Close();


	m_wvp = new CFloat4x4Buffer(m_videoDevice, errorLog, m_commandList);

	m_floats = new CFloatBuffer(m_videoDevice, errorLog, m_commandList);

	m_ints = new CIntBuffer(m_videoDevice, errorLog, m_commandList);

	m_shaderHeap = new CShaderHeap(m_videoDevice, errorLog, 10);


	m_overheadWvp = new CFloat4x4Buffer(m_videoDevice, errorLog, m_commandList);

	m_overheadFloats = new CFloatBuffer(m_videoDevice, errorLog, m_commandList);

	m_overheadInts = new CIntBuffer(m_videoDevice, errorLog, m_commandList);

	m_overheadHeap = new CShaderHeap(m_videoDevice, errorLog, 10);


	m_texture0 = textureManager->Create("image\\grass_03_d.tga");
	m_texture1 = textureManager->Create("image\\rock_01_d.tga");
	m_texture2 = textureManager->Create("image\\dirt_01_d.tga");
	m_texture3 = textureManager->Create("image\\mask1.bmp");
	m_texture4 = textureManager->Create("image\\mask2.bmp");
	m_texture5 = textureManager->Create("image\\lightmap.bmp");

	m_shaderHeap->SlotResource(0, m_wvp);
	m_shaderHeap->SlotResource(1, m_floats);
	m_shaderHeap->SlotResource(2, m_ints);
	m_shaderHeap->SlotResource(3, m_texture0, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_shaderHeap->SlotResource(4, m_texture1, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_shaderHeap->SlotResource(5, m_texture2, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_shaderHeap->SlotResource(6, m_texture3, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_shaderHeap->SlotResource(7, m_texture4, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_shaderHeap->SlotResource(8, m_texture5, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

	m_overheadHeap->SlotResource(0, m_overheadWvp);
	m_overheadHeap->SlotResource(1, m_overheadFloats);
	m_overheadHeap->SlotResource(2, m_overheadInts);
	m_overheadHeap->SlotResource(3, m_texture0, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_overheadHeap->SlotResource(4, m_texture1, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_overheadHeap->SlotResource(5, m_texture2, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_overheadHeap->SlotResource(6, m_texture3, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_overheadHeap->SlotResource(7, m_texture4, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	m_overheadHeap->SlotResource(8, m_texture5, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);


	m_descriptorRange = new CDescriptorRange(2);


	m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3);
	m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7);


	m_rootSignature = new CRootSignature(m_videoDevice, errorLog,
		D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
		2,
		m_descriptorRange);

	m_pipelineState = new CPipelineState(m_videoDevice, errorLog, CVertex::E_VT_VERTEXNT);

	m_vertexShader = shaderManager->Create("vertexTerrain.hlsl", "VSMain", "vs_5_1");
	m_pixelShader =  shaderManager->Create("pixelTerrain.hlsl", "PSMain", "ps_5_1");

	m_pipelineState->Create(false, true, true,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		m_vertexShader, m_pixelShader, nullptr, nullptr, nullptr,
		m_rootSignature->m_signature);

	
	m_shadowMap = new CShadowMap(errorLog, m_videoDevice, shaderManager, m_rootSignature, m_shaderHeap, 9);


	FILE* file = {};

	errno_t err = fopen_s(&file, filename, "rb");

	if (err)
	{
		errorLog->WriteError("CTerrain::CTerrain:Terrain not found:%s\n", filename);

		return;
	}

	size_t bytesRead = 0;

	bytesRead = fread_s(&m_width, sizeof(int), sizeof(int), 1, file);
	bytesRead = fread_s(&m_height, sizeof(int), sizeof(int), 1, file);
	bytesRead = fread_s(&m_primSize, sizeof(int), sizeof(int), 1, file);


	CVertexNT* vertices = (CVertexNT*)malloc(sizeof(CVertexNT) * m_width * m_height);

	if (vertices)
	{
		bytesRead = fread_s(vertices, sizeof(CVertexNT) * m_width * m_height, sizeof(CVertexNT), (size_t)m_width * m_height, file);
	}

	fclose(file);


	m_vertexBuffer = new CVertexBuffer(m_videoDevice, errorLog, m_commandList, CVertex::E_VT_VERTEXNT, m_width * m_height, (void*)vertices);

	free(vertices);

	
	// 6 indices for each tile
	// number of tiles is one less than the width and height of the heightmap
	CHeapArray* indices = new CHeapArray(sizeof(UINT) * 6, true, false, 2, (m_width - 1), (m_height - 1));

	for (UINT h = 0; h < m_height - 1;h++)
	{
		for (UINT w = 0; w < m_width - 1;w++)
		{
			UINT* i = (UINT*)indices->GetElement(2, w, h);

			i[0] = w + (h * (m_width));
			i[1] = w + ((h + 1) * (m_width));
			i[2] = w + 1 + (h * (m_width));
			
			i[3] = w + 1 + (h * (m_width));
			i[4] = w + ((h + 1) * (m_width));
			i[5] = w + 1 + ((h + 1) * (m_width));
		}
	}
	
	m_indexBuffer = new CIndexBuffer(m_videoDevice, errorLog, m_commandList, (m_width - 1) * (m_height - 1) * 6, (void*)indices->m_heap);

	delete indices;
}

/*
*/
CTerrain::~CTerrain()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;
	delete m_pipelineState;
	delete m_rootSignature;
	delete m_descriptorRange;
	delete m_shaderHeap;
	delete m_shadowMap;
	delete m_ints;
	delete m_floats;
	delete m_wvp;

	delete m_overheadWvp;
	delete m_overheadInts;
	delete m_overheadFloats;
	delete m_overheadHeap;

	m_commandList.Reset();

	for (int i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}

	delete m_name;
}

/*
*/
void CTerrain::DrawGeometry()
{
	m_indexBuffer->Draw();

	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);
}

/*
*/
void CTerrain::Record()
{
	m_commandAllocators[m_videoDevice->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_videoDevice->m_backbufferIndex].Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_videoDevice->m_swapChainViewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &m_videoDevice->m_swapChainViewport->m_scissorRect);

	
	m_wvp->UpdateConstantBuffer();

	m_floats->UpdateConstantBuffer();

	m_ints->UpdateConstantBuffer();


	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetGraphicsRootSignature(m_rootSignature->m_signature.Get());

	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap->m_heap->GetGPUDescriptorHandleForHeapStart());


	m_shadowMap->PreRecord(m_commandList);

	CTerrain::DrawGeometry();

	m_shadowMap->PostRecord(m_commandList);

	
	m_commandList->OMSetRenderTargets(1,
		&m_videoDevice->m_swapChainRenderTargets[m_videoDevice->m_backbufferIndex]->m_handle,
		false,
		&m_videoDevice->m_swapChainDepthBuffers[m_videoDevice->m_backbufferIndex]->m_handle);

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	CTerrain::DrawGeometry();
}

/*
*/
void CTerrain::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, &overhead->m_viewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &overhead->m_viewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&overhead->m_renderTargets[m_videoDevice->m_backbufferIndex]->m_handle,
		false,
		&overhead->m_depthBuffers[m_videoDevice->m_backbufferIndex]->m_handle);


	m_overheadWvp->UpdateConstantBuffer();

	m_overheadFloats->UpdateConstantBuffer();

	m_overheadInts->UpdateConstantBuffer();


	m_commandList->SetDescriptorHeaps(1, m_overheadHeap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadHeap->m_heap->GetGPUDescriptorHandleForHeapStart());


	CTerrain::DrawGeometry();
}

/*
*/
void CTerrain::SetCurrentCamera(CCamera* camera)
{
	XMStoreFloat4x4(&m_wvp->m_values[0], camera->m_xmworld);
	XMStoreFloat4x4(&m_wvp->m_values[1], camera->m_xmview);
	XMStoreFloat4x4(&m_wvp->m_values[2], camera->m_xmproj);
}

/*
*/
void CTerrain::SetOverheadCamera(CCamera* camera)
{
	XMStoreFloat4x4(&m_overheadWvp->m_values[0], camera->m_xmworld);
	XMStoreFloat4x4(&m_overheadWvp->m_values[1], camera->m_xmview);
	XMStoreFloat4x4(&m_overheadWvp->m_values[2], camera->m_xmproj);
}