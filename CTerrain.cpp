#include "CTerrain.h"

/*
*/
CTerrain::CTerrain()
{
	memset(this, 0x00, sizeof(CTerrain));
}

/*
*/
CTerrain::CTerrain(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager,
	CEntity* entity, CShadowMap* shadowMap, const char* name)
{
	memset(this, 0x00, sizeof(CTerrain));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_textureManager = textureManager;

	m_shaderBinaryManager = shaderBinaryManager;

	m_entity = entity;

	m_shadowMap = shadowMap;

	m_name = new CString(name);

	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(m_name->GetWide());

	m_commandList->Close();

	m_entity->GetKeyValue("compile", &m_compile);
	m_entity->GetKeyValue("width", &m_width);
	m_entity->GetKeyValue("height", &m_height);

	m_filename = new CString(m_local->m_installPath->m_text);

	m_filename->Append(m_local->m_assetDirectory->m_text);
	m_filename->Append(m_compile);

	m_err = fopen_s(&m_fTerrain, m_filename->m_text, "rb");

	if (m_err)
	{
		m_errorLog->WriteError("CTerrain::CTerrain:Terrain not found:%s\n", m_filename->m_text);

		return;
	}

	m_vertices = new CVertexNT[m_width * m_height]();

	fread_s(m_vertices, sizeof(CVertexNT) * m_width * m_height, sizeof(CVertexNT), (size_t)m_width * m_height, m_fTerrain);

	fclose(m_fTerrain);

	m_vertexBuffer = new CVertexBuffer();

	m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, VertexType::E_VT_VERTEXNT, m_width * m_height, (void*)m_vertices);

	delete[] m_vertices;

	m_indices = new CHeapArray((UINT)sizeof(UINT) * 6, true, false, 2, (m_width - 1), (m_height - 1));

	for (int h = 0; h < m_height - 1;h++)
	{
		for (int w = 0; w < m_width - 1;w++)
		{
			UINT* i = (UINT*)m_indices->GetElement(2, w, h);

			i[0] = w + (h * (m_width));
			i[1] = w + ((h + 1) * (m_width));
			i[2] = w + 1 + (h * (m_width));
			
			i[3] = w + 1 + (h * (m_width));
			i[4] = w + ((h + 1) * (m_width));
			i[5] = w + 1 + ((h + 1) * (m_width));
		}
	}
	
	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, m_errorLog, (m_width - 1) * (m_height - 1) * 6, (void*)m_indices->m_heap);

	delete m_indices;

	m_vs = m_shaderBinaryManager->Get("Terrain.vs");
	m_ps = m_shaderBinaryManager->Get("Terrain.ps");

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Terrain");

	m_shader->AllocateConstantBuffers(3);

	m_shader->SetTextureCount(7);

	m_shader->CreateConstantBuffer(m_shader->m_b[0], sizeof(XMFLOAT4X4) * 5 + sizeof(XMFLOAT4) * 2, m_commandList);
	m_shader->CreateConstantBuffer(m_shader->m_b[1], 104, m_commandList);
	m_shader->CreateConstantBuffer(m_shader->m_b[2], 96, m_commandList);

	m_shader->AllocateDescriptorRange(2);

	m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
	m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

	m_shader->AllocateRootParameter();

	m_shader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

	m_shader->BindConstantBuffers();

	m_shader->BindResource(m_shader->m_t[0], m_shadowMap->m_texture->m_texture.Get(), &m_shadowMap->m_texture->m_srvDesc);

	m_overheadShader = new CShader();

	m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, "OverheadTerrain");

	m_overheadShader->AllocateConstantBuffers(3);

	m_overheadShader->SetTextureCount(7);

	m_overheadShader->CreateConstantBuffer(m_overheadShader->m_b[0], m_shader->m_constantBuffer[0].m_size, m_commandList);
	m_overheadShader->CreateConstantBuffer(m_overheadShader->m_b[1], m_shader->m_constantBuffer[1].m_size, m_commandList);
	m_overheadShader->CreateConstantBuffer(m_overheadShader->m_b[2], m_shader->m_constantBuffer[2].m_size, m_commandList);

	m_overheadShader->AllocateDescriptorRange(2);

	m_overheadShader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_overheadShader->m_constantBufferCount);
	m_overheadShader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_overheadShader->m_textureCount);

	m_overheadShader->AllocateRootParameter();

	m_overheadShader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

	m_overheadShader->BindConstantBuffers();

	m_overheadShader->BindResource(m_overheadShader->m_t[0], m_shadowMap->m_texture->m_texture.Get(), &m_shadowMap->m_texture->m_srvDesc);


	for (int i = 0; i < CTerrain::MAX_TEXTURE; i++)
	{
		memset(m_stringBuffer, 0x00, CTerrain::MAX_STRING_BUFFER);

		sprintf_s(m_stringBuffer, CTerrain::MAX_STRING_BUFFER, "texture%i", i);

		m_entity->GetKeyValue(m_stringBuffer, &m_textureValue);

		if (strlen(m_textureValue) > 0)
		{
			sscanf_s(m_textureValue, "%i %i %s", &m_scaleX, &m_scaleY, &m_textureName, CTerrain::MAX_TEXTURE_NAME);

			m_shader->m_constantBuffer[m_shader->m_b[2]].m_values->Append(i);
			m_shader->m_constantBuffer[m_shader->m_b[2]].m_values->Append(m_scaleX);
			m_shader->m_constantBuffer[m_shader->m_b[2]].m_values->Append(m_scaleY);

			m_overheadShader->m_constantBuffer[m_overheadShader->m_b[2]].m_values->Append(i);
			m_overheadShader->m_constantBuffer[m_overheadShader->m_b[2]].m_values->Append(m_scaleX);
			m_overheadShader->m_constantBuffer[m_overheadShader->m_b[2]].m_values->Append(m_scaleY);

			m_texture[i] = m_textureManager->Create(m_textureName);

			m_shader->BindResource(m_shader->m_t[i + 1], m_texture[i]->m_texture.Get(), &m_texture[i]->m_srvDesc);

			m_overheadShader->BindResource(m_shader->m_t[i + 1], m_texture[i]->m_texture.Get(), &m_texture[i]->m_srvDesc);
		}
	}

	m_isInitialized = true;
}

/*
*/
CTerrain::~CTerrain()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;

	m_commandList.Reset();

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}

	delete m_overheadShader;
	delete m_shader;
	delete m_name;
}

/*
*/
void CTerrain::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_indexBuffer->SetCommandList(commandList);
	m_indexBuffer->Draw();

	m_vertexBuffer->SetCommandList(commandList);
	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);
}

/*
*/
void CTerrain::Record()
{
	m_commandAllocators[m_graphicsAdapter->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_graphicsAdapter->m_backbufferIndex].Get(), nullptr);

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

	CTerrain::DrawGeometry(m_commandList);
}

/*
*/
void CTerrain::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, &overhead->m_viewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &overhead->m_viewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&overhead->m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		false,
		&overhead->m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle);

	m_overheadShader->UpdateConstantBuffers();

	m_commandList->SetDescriptorHeaps(1, m_overheadShader->m_heap->m_heap.GetAddressOf());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShader->m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

	CTerrain::DrawGeometry(m_commandList);
}

/*
*/
void CTerrain::SetCurrentCamera(CCamera* camera)
{
	m_shader->m_constantBuffer[m_shader->m_b[0]].Reset();

	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(camera->m_xmworld);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(camera->m_xmview);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(camera->m_xmproj);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_position.m_p);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(1.0f);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_direction.m_p);
	m_shader->m_constantBuffer[m_shader->m_b[0]].m_values->Append(1.0f);
}

/*
*/
void CTerrain::SetOverheadCamera(CCamera* camera)
{
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].Reset();

	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(camera->m_xmworld);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(camera->m_xmview);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(camera->m_xmproj);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_position.m_p);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(1.0f);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_direction.m_p);
	m_overheadShader->m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(1.0f);
}