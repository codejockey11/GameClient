#include "CTerrain.h"

/*
*/
CTerrain::CTerrain()
{
	memset(this, 0x00, sizeof(CTerrain));
}

/*
*/
CTerrain::CTerrain(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager, int32_t bundle,
	CEntity* entity, CShadowMap* shadowMap, CVec3f* ambient, const char* name)
{
	memset(this, 0x00, sizeof(CTerrain));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_textureManager = textureManager;

	m_shaderBinaryManager = shaderBinaryManager;

	m_bundle = bundle;

	m_entity = entity;

	m_shadowMap = shadowMap;

	m_name = new CString(name);

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(m_name->GetWide());

	m_commandList->Close();

	m_entity->GetKeyValue("compile", &m_compile);
	m_entity->GetKeyValue("width", &m_width);
	m_entity->GetKeyValue("depth", &m_depth);

	m_filename = new CString(m_local->m_installPath->m_text);

	m_filename->Append(m_local->m_assetDirectory->m_text);
	m_filename->Append(m_compile);

	m_err = fopen_s(&m_fTerrain, m_filename->m_text, "rb");

	if (m_err)
	{
		m_errorLog->WriteError("CTerrain::CTerrain:Terrain not found:%s\n", m_filename->m_text);

		return;
	}

	m_vertices = new CVertexNT[m_width * m_depth]();

	fread_s(m_vertices, sizeof(CVertexNT) * m_width * m_depth, sizeof(CVertexNT), (size_t)m_width * m_depth, m_fTerrain);

	fclose(m_fTerrain);

	m_vertexBuffer = new CVertexBuffer();

	m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, VertexType::E_VT_VERTEXNT, m_width * m_depth, (void*)m_vertices);

	SAFE_DELETE_ARRAY(m_vertices);

	m_indices = new CHeapArray(true, (int32_t)sizeof(uint16_t) * 6, 2, m_width, m_depth);

	for (uint16_t h = 0; h < m_depth - 1; h++)
	{
		for (uint16_t w = 0; w < m_width - 1; w++)
		{
			uint16_t* i = (uint16_t*)m_indices->GetElement(2, w, h);

			i[0] = w + (h * (uint16_t)m_width);
			i[1] = w + ((h + 1) * (uint16_t)m_width);
			i[2] = w + 1 + (h * (uint16_t)m_width);

			i[3] = w + 1 + (h * (uint16_t)m_width);
			i[4] = w + ((h + 1) * (uint16_t)m_width);
			i[5] = w + 1 + ((h + 1) * (uint16_t)m_width);
		}
	}

	m_indexBuffer = new CIndexBuffer(m_graphicsAdapter, m_errorLog, m_width * m_depth * 6, (void*)m_indices->m_heap);

	SAFE_DELETE(m_indices);

	m_vs = m_shaderBinaryManager->Get("terrain.vs");
	m_ps = m_shaderBinaryManager->Get("terrain.ps");
	
	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->AllocateSignature(4, 11);

	m_pipelineState->SetPixelShader(m_ps);
	m_pipelineState->SetVertexShader(m_vs);

	m_pipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_cameraConstantBuffer = new CConstantBuffer();

	m_cameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 384);

	m_overheadCameraConstantBuffer = new CConstantBuffer();

	m_overheadCameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 384);

	m_materialConstantBuffer = new CConstantBuffer();

	m_materialConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 104);

	m_materialConstantBuffer->m_values->Append(ambient);
	m_materialConstantBuffer->m_values->Append(1.0f);

	m_lightConstantBuffer = new CConstantBuffer();

	m_lightConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 1168);

	if (m_lights)
	{
		m_lightConstantBuffer->Reset();

		m_lightConstantBuffer->m_values->Append(m_lights->m_count);
		m_lightConstantBuffer->m_values->Append(0);
		m_lightConstantBuffer->m_values->Append(0);
		m_lightConstantBuffer->m_values->Append(0);

		m_node = m_lights->m_list;

		while ((m_node) && (m_node->m_object))
		{
			m_light = (CLight*)m_node->m_object;

			m_lightConstantBuffer->m_values->Append(&m_light->m_position);
			m_lightConstantBuffer->m_values->Append(1.0f);
			m_lightConstantBuffer->m_values->Append(&m_light->m_direction);
			m_lightConstantBuffer->m_values->Append(1.0f);
			m_lightConstantBuffer->m_values->Append(&m_light->m_color);
			m_lightConstantBuffer->m_values->Append(m_light->m_radius);

			m_node = m_node->m_next;
		}
	}

	m_blendConstantBuffer = new CConstantBuffer();

	m_blendConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 128);

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Terrain");

	m_shader->SetConstantBufferCount(4);
	m_shader->SetTextureCount(11);

	m_shader->AllocateHeap();

	m_shader->BindResource(0, m_cameraConstantBuffer->m_buffer.Get(), &m_cameraConstantBuffer->m_srvDesc);
	m_shader->BindResource(1, m_materialConstantBuffer->m_buffer.Get(), &m_materialConstantBuffer->m_srvDesc);
	m_shader->BindResource(2, m_lightConstantBuffer->m_buffer.Get(), &m_lightConstantBuffer->m_srvDesc);
	m_shader->BindResource(3, m_blendConstantBuffer->m_buffer.Get(), &m_blendConstantBuffer->m_srvDesc);

	m_shader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

	m_overheadShader = new CShader();

	m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, "OverheadTerrain");

	m_overheadShader->SetConstantBufferCount(4);
	m_overheadShader->SetTextureCount(11);

	m_overheadShader->AllocateHeap();

	m_overheadShader->BindResource(0, m_overheadCameraConstantBuffer->m_buffer.Get(), &m_overheadCameraConstantBuffer->m_srvDesc);
	m_overheadShader->BindResource(1, m_materialConstantBuffer->m_buffer.Get(), &m_materialConstantBuffer->m_srvDesc);
	m_overheadShader->BindResource(2, m_lightConstantBuffer->m_buffer.Get(), &m_lightConstantBuffer->m_srvDesc);
	m_overheadShader->BindResource(3, m_blendConstantBuffer->m_buffer.Get(), &m_blendConstantBuffer->m_srvDesc);

	m_overheadShader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

	for (int32_t i = 0; i < CTerrain::MAX_TEXTURE; i++)
	{
		memset(m_stringBuffer, 0x00, CTerrain::MAX_STRING_BUFFER);

		sprintf_s(m_stringBuffer, CTerrain::MAX_STRING_BUFFER, "texture%i", i);

		m_entity->GetKeyValue(m_stringBuffer, &m_textureValue);

		if (strlen(m_textureValue) > 0)
		{
			sscanf_s(m_textureValue, "%i %i %s", &m_scaleX, &m_scaleY, &m_textureName, CTerrain::MAX_TEXTURE_NAME);

			m_blendConstantBuffer->m_values->Append(i);
			m_blendConstantBuffer->m_values->Append(m_scaleX);
			m_blendConstantBuffer->m_values->Append(m_scaleY);
			m_blendConstantBuffer->m_values->Append(0);

			m_texture[i] = m_textureManager->Create(m_textureName);

			m_shader->BindResource(m_shader->m_t[i + 1], m_texture[i]->m_texture.Get(), &m_texture[i]->m_srvDesc);

			m_overheadShader->BindResource(m_overheadShader->m_t[i + 1], m_texture[i]->m_texture.Get(), &m_texture[i]->m_srvDesc);
		}
	}

	m_isInitialized = true;
}

/*
*/
CTerrain::~CTerrain()
{
	SAFE_DELETE(m_overheadShader);
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_overheadCameraConstantBuffer);
	SAFE_DELETE(m_cameraConstantBuffer);
	SAFE_DELETE(m_materialConstantBuffer);
	SAFE_DELETE(m_materialConstantBuffer);
	SAFE_DELETE(m_lightConstantBuffer);
	SAFE_DELETE(m_blendConstantBuffer);
	SAFE_DELETE(m_pipelineState);
	SAFE_DELETE(m_indexBuffer);
	SAFE_DELETE(m_vertexBuffer);
	SAFE_DELETE(m_filename);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
	SAFE_DELETE(m_name);
}

/*
*/
void CTerrain::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_indexBuffer->SetCommandList(commandList);
	m_indexBuffer->Record();

	m_vertexBuffer->SetCommandList(commandList);
	m_vertexBuffer->DrawIndexed(m_indexBuffer->m_count);
}

/*
*/
void CTerrain::Record()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	m_commandList->RSSetViewports(1, m_graphicsAdapter->GetViewport());

	m_commandList->RSSetScissorRects(1, m_graphicsAdapter->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, m_graphicsAdapter->GetRenderTarget(), false, m_graphicsAdapter->GetDepthBuffer());

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());
	
	m_cameraConstantBuffer->UpdateBuffer();

	m_materialConstantBuffer->UpdateBuffer();

	m_lightConstantBuffer->UpdateBuffer();

	m_blendConstantBuffer->UpdateBuffer();

	m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

	CTerrain::DrawGeometry(m_commandList);

	m_graphicsAdapter->BundleCommandList(m_bundle, m_commandList);
}

/*
*/
void CTerrain::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, overhead->GetViewport());

	m_commandList->RSSetScissorRects(1, overhead->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, overhead->GetRenderTarget(), false, overhead->GetDepthBuffer());

	m_overheadCameraConstantBuffer->UpdateBuffer();

	m_commandList->SetDescriptorHeaps(1, m_overheadShader->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShader->GetBaseDescriptor());

	CTerrain::DrawGeometry(m_commandList);
}

/*
*/
void CTerrain::SetCurrentCamera(CCamera* camera)
{
	m_cameraConstantBuffer->Reset();

	m_cameraConstantBuffer->m_values->Append(camera->m_xmworld);
	m_cameraConstantBuffer->m_values->Append(camera->m_xmview);
	m_cameraConstantBuffer->m_values->Append(camera->m_xmproj);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
	m_cameraConstantBuffer->m_values->Append(camera->m_position);
	m_cameraConstantBuffer->m_values->Append(1.0f);
	m_cameraConstantBuffer->m_values->Append(camera->m_look);
	m_cameraConstantBuffer->m_values->Append(1.0f);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
	m_cameraConstantBuffer->m_values->Append(1.0f);
	m_cameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
	m_cameraConstantBuffer->m_values->Append(1.0f);
}

/*
*/
void CTerrain::SetLights(CList* lights)
{
	m_lights = lights;

	if (m_lights == nullptr)
	{
		return;
	}

	m_lightConstantBuffer->Reset();

	m_lightConstantBuffer->m_values->Append(m_lights->m_count);
	m_lightConstantBuffer->m_values->Append(0);
	m_lightConstantBuffer->m_values->Append(0);
	m_lightConstantBuffer->m_values->Append(0);

	m_node = m_lights->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_light = (CLight*)m_node->m_object;

		m_lightConstantBuffer->m_values->Append(&m_light->m_position);
		m_lightConstantBuffer->m_values->Append(1.0f);
		m_lightConstantBuffer->m_values->Append(&m_light->m_direction);
		m_lightConstantBuffer->m_values->Append(1.0f);
		m_lightConstantBuffer->m_values->Append(&m_light->m_color);
		m_lightConstantBuffer->m_values->Append(m_light->m_radius);

		m_node = m_node->m_next;
	}
}

/*
*/
void CTerrain::SetOverheadCamera(CCamera* camera)
{
	m_overheadCameraConstantBuffer->Reset();

	m_overheadCameraConstantBuffer->m_values->Append(camera->m_xmworld);
	m_overheadCameraConstantBuffer->m_values->Append(camera->m_xmview);
	m_overheadCameraConstantBuffer->m_values->Append(camera->m_xmproj);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
	m_overheadCameraConstantBuffer->m_values->Append(camera->m_position);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
	m_overheadCameraConstantBuffer->m_values->Append(camera->m_look);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
	m_overheadCameraConstantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
	m_overheadCameraConstantBuffer->m_values->Append(1.0f);
}