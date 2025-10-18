#include "CMap.h"

/*
*/
CMap::CMap()
{
	memset(this, 0x00, sizeof(CMap));
}

/*
*/
CMap::CMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CSoundDevice* soundDevice, CWavefrontManager* wavefrontManager,
	CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager, CSoundManager* soundManager, char* name, CShadowMap* shadowMap)
{
	memset(this, 0x00, sizeof(CMap));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_soundDevice = soundDevice;

	m_wavefrontManager = wavefrontManager;

	m_textureManager = textureManager;

	m_shaderBinaryManager = shaderBinaryManager;

	m_soundManager = soundManager;

	m_name = new CString(name);

	m_filename = new CString(m_local->m_installPath->m_text);
	
	m_filename->Append("main/maps/");
	m_filename->Append(m_name->m_text);
	m_filename->Append(".dat");

	m_shadowMap = shadowMap;

	m_vertexBuffers = new CLinkList<CVertexBuffer>();

	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(m_name->GetWide());

	m_commandList->Close();

	m_err = fopen_s(&m_fMap, m_filename->m_text, "rb");

	if (m_err)
	{
		m_errorLog->WriteError("CMap::CMap:Map not found:%s\n", m_filename->m_text);

		return;
	}

	fread_s(&m_numberOfMaterials, sizeof(int), sizeof(int), 1, m_fMap);

	m_shaderMaterial = new CShaderMaterial[m_numberOfMaterials]();

	fread_s(m_shaderMaterial, sizeof(CShaderMaterial) * m_numberOfMaterials, sizeof(CShaderMaterial), m_numberOfMaterials, m_fMap);

	fread_s(&m_maxEntityCount, sizeof(int), sizeof(int), 1, m_fMap);

	m_entity = new CEntity[m_maxEntityCount]();

	while (!feof(m_fMap))
	{
		fread_s(&m_entity[m_entityCount].m_number, sizeof(int), sizeof(int), 1, m_fMap);
		fread_s(&m_entity[m_entityCount].m_type, sizeof(unsigned char), sizeof(unsigned char), 1, m_fMap);

		fread_s(&m_keyValueCount, sizeof(int), sizeof(int), 1, m_fMap);

		m_entity[m_entityCount].Initialize(m_keyValueCount);

		for (int i = 0; i < m_keyValueCount; i++)
		{
			fread_s(m_key, CKeyValue::MAX_KEY, sizeof(char), CKeyValue::MAX_KEY, m_fMap);
			fread_s(m_value, CKeyValue::MAX_VALUE, sizeof(char), CKeyValue::MAX_VALUE, m_fMap);

			m_entity[m_entityCount].AddKeyValue(m_key, m_value);
		}

		switch (m_entity[m_entityCount].m_type)
		{
		case CEntity::Type::WORLDSPAWN:
		{
			m_entity[m_entityCount].GetKeyValue("mapSize", &m_mapSize);
			m_entity[m_entityCount].GetKeyValue("sectorSize", &m_sectorSize);

			fread_s(&m_materialNumber, sizeof(int), sizeof(int), 1, m_fMap);

			while (m_materialNumber != -1)
			{
				fread_s(&m_verticesCount, sizeof(int), sizeof(int), 1, m_fMap);

				m_vertices = new CVertexNT[m_verticesCount]();

				fread_s(m_vertices, sizeof(CVertexNT) * m_verticesCount, sizeof(CVertexNT), m_verticesCount, m_fMap);

				m_vertexBuffer = new CVertexBuffer();

				m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_materialNumber, VertexType::E_VT_VERTEXNT, m_verticesCount, (void*)m_vertices);

				m_vertexBuffer->SetCommandList(m_commandList);

				m_vertexBuffers->Append(m_vertexBuffer, m_vertexBufferCount);

				delete[] m_vertices;

				m_vertexBufferCount++;

				fread_s(&m_materialNumber, sizeof(int), sizeof(int), 1, m_fMap);
			}

			break;
		}
		case CEntity::Type::COLLECTABLE:
		{
			if (m_visibility == nullptr)
			{
				m_visibility = new CVisibility(m_mapSize.m_p.x, m_mapSize.m_p.z, m_mapSize.m_p.y, m_sectorSize);
			}

			m_entity[m_entityCount].GetKeyValue("origin", &m_vec3f);

			m_float = m_vec3f.m_p.y;
			m_vec3f.m_p.y = m_vec3f.m_p.z;
			m_vec3f.m_p.z = m_float;

			m_entity[m_entityCount].GetKeyValue("model", &m_objectName);

			m_wavefront = m_wavefrontManager->Create(m_objectName);

			m_entity[m_entityCount].GetKeyValue("name", &m_objectName);

			m_object = new CObject();

			m_object->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_objectName, m_wavefront, m_textureManager, m_shaderBinaryManager, "Collectable", true, m_shadowMap, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

			m_object->SetPosition(&m_vec3f);
			m_object->Update();

			m_wavLoader = m_soundManager->Create("audio/mono/rifle.wav");

			m_object->AddSound(m_wavLoader, false);

			m_wavLoader = m_soundManager->Create("audio/mono/shotgun.wav");

			m_object->AddSound(m_wavLoader, false);

			m_object->m_sounds[0]->SetPosition(&m_object->m_position);
			m_object->m_sounds[1]->SetPosition(&m_object->m_position);

			m_object->m_sounds[0]->SetRange(256.0);
			m_object->m_sounds[1]->SetRange(256.0);

			m_entity[m_entityCount].GetKeyValue("scale", &m_vec3f);

			m_object->SetScale(&m_vec3f);

			m_entity[m_entityCount].GetKeyValue("amplitude", &m_float);

			m_object->SetAmplitude(m_float);

			m_entity[m_entityCount].GetKeyValue("spin", &m_float);

			m_object->SetSpin(m_float);

			m_entity[m_entityCount].GetKeyValue("frequency", &m_float);

			m_object->SetFrequency(m_float);

			m_entity[m_entityCount].GetKeyValue("diffuse", &m_vec3f);

			m_object->SetDiffuse(&m_vec3f);

			m_object->InitMaterialBuffer();

			m_index = m_visibility->m_sector->GetSector(&m_object->m_position);

			m_sector = (CLinkList<CObject>*)m_visibility->m_collectables->GetElement(2, m_index.m_p.x, m_index.m_p.z);

			if (m_sector != nullptr)
			{
				if (m_sector->m_list == nullptr)
				{
					m_sector->Constructor();
				}

				m_sector->Add(m_object, m_object->m_name->m_text);
			}

			break;
		}
		case CEntity::Type::STATICMODEL:
		{
			if (m_visibility == nullptr)
			{
				m_visibility = new CVisibility(m_mapSize.m_p.x, m_mapSize.m_p.z, m_mapSize.m_p.y, m_sectorSize);
			}

			m_entity[m_entityCount].GetKeyValue("origin", &m_vec3f);

			m_float = m_vec3f.m_p.y;
			m_vec3f.m_p.y = m_vec3f.m_p.z;
			m_vec3f.m_p.z = m_float;

			m_entity[m_entityCount].GetKeyValue("model", &m_objectName);

			m_wavefront = m_wavefrontManager->Create(m_objectName);

			m_entity[m_entityCount].GetKeyValue("name", &m_objectName);

			m_object = new CObject();

			m_object->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_objectName, m_wavefront, m_textureManager, m_shaderBinaryManager, "Static", true, m_shadowMap, D3D12_CULL_MODE::D3D12_CULL_MODE_NONE);

			m_object->SetPosition(&m_vec3f);
			m_object->Update();

			m_entity[m_entityCount].GetKeyValue("scale", &m_vec3f);

			m_object->SetScale(&m_vec3f);

			m_entity[m_entityCount].GetKeyValue("direction", &m_vec3f);

			m_object->SetRotation(&m_vec3f);

			m_xmfloat4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			m_object->SetDiffuse(&m_xmfloat4);

			m_object->InitMaterialBuffer();

			m_index = m_visibility->m_sector->GetSector(&m_object->m_position);

			m_sector = (CLinkList<CObject>*)m_visibility->m_statics->GetElement(2, m_index.m_p.x, m_index.m_p.z);

			if (m_sector != nullptr)
			{
				if (m_sector->m_list == nullptr)
				{
					m_sector->Constructor();
				}

				m_sector->Add(m_object, m_object->m_name->m_text);
			}

			break;
		}
		case CEntity::Type::TERRAIN:
		{
			m_terrain = new CTerrain(m_graphicsAdapter, m_errorLog, m_local, m_textureManager, m_shaderBinaryManager, &m_entity[m_entityCount], m_shadowMap, name);

			break;
		}
		default:
		{
			m_entity[m_entityCount].GetKeyValue("classname", &m_objectName);

			m_errorLog->WriteError(true, "CMap::CMap:Unhandled entity type:%s\n", m_objectName);

			break;
		}
		}

		m_entityCount++;

		if (m_entityCount == m_maxEntityCount)
		{
			break;
		}
	}

	fclose(m_fMap);

	m_vs = m_shaderBinaryManager->Get("Map.vs");
	m_ps = m_shaderBinaryManager->Get("Map.ps");

	m_shader = new CShader[m_numberOfMaterials]();

	m_overheadShader = new CShader[m_numberOfMaterials]();

	for (int i = 0; i < m_numberOfMaterials; i++)
	{
		m_shader[i].Constructor(m_graphicsAdapter, m_errorLog, "Map");

		m_shader[i].AllocateConstantBuffers(3);

		m_shader[i].CreateConstantBuffer(m_shader->m_b[0], sizeof(XMFLOAT4X4) * 5, m_commandList);
		m_shader[i].CreateConstantBuffer(m_shader->m_b[1], 104, m_commandList);
		m_shader[i].CreateConstantBuffer(m_shader->m_b[2], 272, m_commandList);

		m_shader->SetTextureCount(3);

		m_shader[i].AllocateDescriptorRange(2);

		m_shader[i].m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
		m_shader[i].m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

		m_shader[i].AllocateRootParameter();

		m_shader[i].AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

		m_shader[i].BindConstantBuffers();

		m_shader[i].BindResource(m_shader->m_t[0], m_shadowMap->m_texture->m_texture.Get(), &m_shadowMap->m_texture->m_srvDesc);

		m_texture = m_textureManager->Create(m_shaderMaterial[i].m_map_Kd);

		m_shader[i].BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

		m_overheadShader[i].Constructor(m_graphicsAdapter, m_errorLog, "MapOverhead");

		m_overheadShader[i].AllocateConstantBuffers(3);

		m_overheadShader[i].CreateConstantBuffer(m_overheadShader->m_b[0], sizeof(XMFLOAT4X4) * 5, m_commandList);
		m_overheadShader[i].CreateConstantBuffer(m_overheadShader->m_b[1], 104, m_commandList);
		m_overheadShader[i].CreateConstantBuffer(m_overheadShader->m_b[2], 272, m_commandList);

		m_overheadShader->SetTextureCount(3);

		m_overheadShader[i].AllocateDescriptorRange(2);

		m_overheadShader[i].m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_overheadShader->m_constantBufferCount);
		m_overheadShader[i].m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_overheadShader->m_textureCount);

		m_overheadShader[i].AllocateRootParameter();

		m_overheadShader[i].AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

		m_overheadShader[i].BindConstantBuffers();

		m_overheadShader[i].BindResource(m_shader->m_t[0], m_shadowMap->m_texture->m_texture.Get(), &m_shadowMap->m_texture->m_srvDesc);

		m_overheadShader[i].BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

		m_shader[i].m_constantBuffer[m_shader->m_b[1]].Reset();

		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_ambient);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_diffuse);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_emissive);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specular);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_illum);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opacity);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specularExponent);
		m_shader[i].m_constantBuffer[m_shader->m_b[1]].m_values->Append(1);

		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].Reset();

		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_ambient);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_diffuse);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_emissive);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specular);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_illum);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opacity);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specularExponent);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[1]].m_values->Append(1);
	}
}

/*
*/
CMap::~CMap()
{
	if (m_terrain)
	{
		delete m_terrain;
	}

	delete m_visibility;

	delete[] m_entity;
	delete[] m_shaderMaterial;
	delete[] m_shader;
	delete[] m_overheadShader;

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}

	m_commandList.Reset();

	delete m_vertexBuffers;
	delete m_name;
	delete m_filename;
}

/*
*/
void CMap::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_vertexBufferNode = m_vertexBuffers->m_list;

	while (m_vertexBufferNode->m_object)
	{
		m_vertexBufferNode->m_object->SetCommandList(commandList);
		m_vertexBufferNode->m_object->Draw();

		m_vertexBufferNode = m_vertexBufferNode->m_next;
	}
}

/*
*/
void CMap::Record()
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

	m_vertexBufferNode = m_vertexBuffers->m_list;

	while (m_vertexBufferNode->m_object)
	{
		m_shader[m_vertexBufferNode->m_object->m_material].UpdateConstantBuffers();

		m_commandList->SetGraphicsRootSignature(m_shader[m_vertexBufferNode->m_object->m_material].m_rootSignature->m_signature.Get());

		m_commandList->SetDescriptorHeaps(1, m_shader[m_vertexBufferNode->m_object->m_material].m_heap->m_heap.GetAddressOf());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shader[m_vertexBufferNode->m_object->m_material].m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->SetPipelineState(m_shader[m_vertexBufferNode->m_object->m_material].m_pipelineState->m_pipelineState.Get());

		m_vertexBufferNode->m_object->SetCommandList(m_commandList);
		m_vertexBufferNode->m_object->Draw();

		m_vertexBufferNode = m_vertexBufferNode->m_next;
	}
}

/*
*/
void CMap::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, &overhead->m_viewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &overhead->m_viewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&overhead->m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		false,
		&overhead->m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle);

	m_vertexBufferNode = m_vertexBuffers->m_list;

	while (m_vertexBufferNode->m_object)
	{
		m_overheadShader[m_vertexBufferNode->m_object->m_material].UpdateConstantBuffers();

		m_commandList->SetGraphicsRootSignature(m_overheadShader[m_vertexBufferNode->m_object->m_material].m_rootSignature->m_signature.Get());

		m_commandList->SetDescriptorHeaps(1, m_overheadShader[m_vertexBufferNode->m_object->m_material].m_heap->m_heap.GetAddressOf());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShader[m_vertexBufferNode->m_object->m_material].m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->SetPipelineState(m_overheadShader[m_vertexBufferNode->m_object->m_material].m_pipelineState->m_pipelineState.Get());

		m_vertexBufferNode->m_object->SetCommandList(m_commandList);
		m_vertexBufferNode->m_object->Draw();

		m_vertexBufferNode = m_vertexBufferNode->m_next;
	}
}

/*
*/
void CMap::SetCurrentCamera(CCamera* camera)
{
	for (int i = 0; i < m_numberOfMaterials; i++)
	{
		m_shader[i].m_constantBuffer[m_shader->m_b[0]].Reset();

		m_shader[i].m_constantBuffer[m_shader->m_b[0]].m_values->Append(camera->m_xmworld);
		m_shader[i].m_constantBuffer[m_shader->m_b[0]].m_values->Append(camera->m_xmview);
		m_shader[i].m_constantBuffer[m_shader->m_b[0]].m_values->Append(camera->m_xmproj);
		m_shader[i].m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
		m_shader[i].m_constantBuffer[m_shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	}
}

/*
*/
void CMap::SetOverheadCamera(CCamera* camera)
{
	for (int i = 0; i < m_numberOfMaterials; i++)
	{
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[0]].Reset();

		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(camera->m_xmworld);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(camera->m_xmview);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(camera->m_xmproj);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
		m_overheadShader[i].m_constantBuffer[m_overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	}
}