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

	m_vertexBuffers = new CList();

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

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

		m_entity[m_entityCount].Constructor(m_keyValueCount);

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
			m_entity[m_entityCount].GetKeyValue("ambient", &m_ambient);

			m_visibility = new CVisibility(m_mapSize.m_p.x, m_mapSize.m_p.z, m_mapSize.m_p.y, m_sectorSize);

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

				SAFE_DELETE_ARRAY(m_vertices);

				m_vertexBufferCount++;

				fread_s(&m_materialNumber, sizeof(int), sizeof(int), 1, m_fMap);
			}

			break;
		}
		case CEntity::Type::COLLECTABLE:
		{
			m_entity[m_entityCount].GetKeyValue("origin", &m_vec3f);

			m_float = m_vec3f.m_p.y;
			m_vec3f.m_p.y = m_vec3f.m_p.z;
			m_vec3f.m_p.z = m_float;

			m_entity[m_entityCount].GetKeyValue("model", &m_objectName);

			m_wavefront = m_wavefrontManager->Create(m_objectName);

			m_entity[m_entityCount].GetKeyValue("name", &m_objectName);

			m_object = new CObject();

			m_object->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_objectName, m_wavefront, m_textureManager, m_shaderBinaryManager,
				"collectable", true, 3, m_shadowMap, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

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

			m_object->SetAmbient(&m_ambient);

			m_object->InitMaterialBuffer();

			m_index = m_visibility->m_sector->GetSector(&m_object->m_position);

			m_sectors = (CList*)m_visibility->m_collectables->GetElement(2, m_index.m_p.x, m_index.m_p.z);

			if (m_sectors->m_list == nullptr)
			{
				m_sectors->Constructor();
			}

			m_sectors->Add(m_object, m_object->m_name->m_text);

			break;
		}
		case CEntity::Type::STATICMODEL:
		{
			m_entity[m_entityCount].GetKeyValue("origin", &m_vec3f);

			m_float = m_vec3f.m_p.y;
			m_vec3f.m_p.y = m_vec3f.m_p.z;
			m_vec3f.m_p.z = m_float;

			m_entity[m_entityCount].GetKeyValue("model", &m_objectName);

			m_wavefront = m_wavefrontManager->Create(m_objectName);

			m_entity[m_entityCount].GetKeyValue("name", &m_objectName);

			m_object = new CObject();

			m_object->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_objectName, m_wavefront, m_textureManager, m_shaderBinaryManager,
				"static", true, 3, m_shadowMap, D3D12_CULL_MODE::D3D12_CULL_MODE_NONE);

			m_object->SetPosition(&m_vec3f);

			m_object->Update();

			m_entity[m_entityCount].GetKeyValue("scale", &m_vec3f);

			m_object->SetScale(&m_vec3f);

			m_entity[m_entityCount].GetKeyValue("direction", &m_vec3f);

			m_object->SetRotation(&m_vec3f);

			m_xmfloat4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			m_object->SetDiffuse(&m_xmfloat4);

			m_object->SetAmbient(&m_ambient);

			m_object->InitMaterialBuffer();

			m_index = m_visibility->m_sector->GetSector(&m_object->m_position);

			m_sectors = (CList*)m_visibility->m_statics->GetElement(2, m_index.m_p.x, m_index.m_p.z);

			if (m_sectors->m_list == nullptr)
			{
				m_sectors->Constructor();
			}

			m_sectors->Add(m_object, m_object->m_name->m_text);

			break;
		}
		case CEntity::Type::TERRAIN:
		{
			m_terrain = new CTerrain(m_graphicsAdapter, m_errorLog, m_local, m_textureManager, m_shaderBinaryManager, 1, &m_entity[m_entityCount], m_shadowMap, &m_ambient, name);

			break;
		}
		case CEntity::Type::LIGHT:
		{
			if (m_lights == nullptr)
			{
				m_lights = new CList();
			}

			m_light = new CLight();

			m_entity[m_entityCount].GetKeyValue("origin", &m_vec3f);

			m_float = m_vec3f.m_p.y;
			m_vec3f.m_p.y = m_vec3f.m_p.z;
			m_vec3f.m_p.z = m_float;

			m_light->m_position = m_vec3f;

			m_entity[m_entityCount].GetKeyValue("light", &m_light->m_radius);
			m_entity[m_entityCount].GetKeyValue("color", &m_light->m_color);

			m_lights->Append(m_light, m_lights->m_count);

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

	if (m_terrain)
	{
		m_terrain->SetLights(m_lights);
	}

	if (m_visibility)
	{
		for (UINT pz = 0; pz < m_visibility->m_sector->m_gridDepth; pz++)
		{
			for (UINT px = 0; px < m_visibility->m_sector->m_gridWidth; px++)
			{
				if (m_visibility->m_collectables)
				{
					m_collectables = (CList*)m_visibility->m_collectables->GetElement(2, px, pz);

					if ((m_collectables) && (m_collectables->m_list))
					{
						m_node = m_collectables->m_list;

						while ((m_node) && (m_node->m_object))
						{
							m_object = (CObject*)m_node->m_object;

							if (!m_object->m_isVisible)
							{
								m_object->SetLights(m_lights);
							}

							m_node = m_node->m_next;
						}
					}
				}

				if (m_visibility->m_statics)
				{
					m_statics = (CList*)m_visibility->m_statics->GetElement(2, px, pz);

					if ((m_statics) && (m_statics->m_list))
					{
						m_node = m_statics->m_list;

						while ((m_node) && (m_node->m_object))
						{
							m_object = (CObject*)m_node->m_object;

							m_object->SetLights(m_lights);

							m_node = m_node->m_next;
						}
					}
				}
			}
		}
	}

	m_vs = m_shaderBinaryManager->Get("map.vs");
	m_ps = m_shaderBinaryManager->Get("map.ps");

	m_shaders = new CShader[m_numberOfMaterials]();

	m_overheadShaders = new CShader[m_numberOfMaterials]();

	for (int i = 0; i < m_numberOfMaterials; i++)
	{
		m_shader = &m_shaders[i];

		m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Map");

		m_shader->AllocateConstantBuffers(4);

		m_shader->SetTextureCount(11);

		m_shader->CreateConstantBuffer(0, 384, m_commandList);
		m_shader->CreateConstantBuffer(1, 104, m_commandList);
		m_shader->CreateConstantBuffer(2, 1168, m_commandList);
		m_shader->CreateConstantBuffer(3, 128, m_commandList);

		m_shader->AllocateDescriptorRange(2);

		m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
		m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

		m_shader->AllocateRootParameter();

		m_shader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

		m_shader->BindConstantBuffers();

		m_texture = m_textureManager->Create(m_shaderMaterial[i].m_map_Kd);

		m_shader->BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

		m_constantBuffer = m_shader->GetConstantBuffer(1);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(&m_ambient);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_diffuse);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_emissive);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_specular);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_illum);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_opacity);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_specularExponent);
		m_constantBuffer->m_values->Append(1);

		if (m_lights)
		{
			m_constantBuffer = m_shader->GetConstantBuffer(2);

			m_constantBuffer->Reset();

			m_constantBuffer->m_values->Append(m_lights->m_count);
			m_constantBuffer->m_values->Append(0);
			m_constantBuffer->m_values->Append(0);
			m_constantBuffer->m_values->Append(0);

			m_node = m_lights->m_list;

			while ((m_node) && (m_node->m_object))
			{
				m_light = (CLight*)m_node->m_object;

				m_constantBuffer->m_values->Append(&m_light->m_position);
				m_constantBuffer->m_values->Append(1.0f);
				m_constantBuffer->m_values->Append(&m_light->m_direction);
				m_constantBuffer->m_values->Append(1.0f);
				m_constantBuffer->m_values->Append(&m_light->m_color);
				m_constantBuffer->m_values->Append(m_light->m_radius);

				m_node = m_node->m_next;
			}
		}

		m_overheadShader = &m_overheadShaders[i];

		m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, "MapOverhead");

		m_overheadShader->AllocateConstantBuffers(4);

		m_overheadShader->SetTextureCount(11);

		m_overheadShader->CreateConstantBuffer(0, 384, m_commandList);
		m_overheadShader->CreateConstantBuffer(1, 104, m_commandList);
		m_overheadShader->CreateConstantBuffer(2, 1168, m_commandList);
		m_overheadShader->CreateConstantBuffer(3, 128, m_commandList);

		m_overheadShader->AllocateDescriptorRange(2);

		m_overheadShader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_overheadShader->m_constantBufferCount);
		m_overheadShader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_overheadShader->m_textureCount);

		m_overheadShader->AllocateRootParameter();

		m_overheadShader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK, m_vs, m_ps);

		m_overheadShader->BindConstantBuffers();

		m_overheadShader->BindResource(m_overheadShader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

		m_constantBuffer = m_overheadShader->GetConstantBuffer(1);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(&m_ambient);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_diffuse);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_emissive);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_specular);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_illum);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_opacity);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		m_constantBuffer->m_values->Append(m_shaderMaterial[i].m_specularExponent);
		m_constantBuffer->m_values->Append(1);

		if (m_lights)
		{
			m_constantBuffer = m_overheadShader->GetConstantBuffer(2);

			m_constantBuffer->Reset();

			m_constantBuffer->m_values->Append(m_lights->m_count);
			m_constantBuffer->m_values->Append(0);
			m_constantBuffer->m_values->Append(0);
			m_constantBuffer->m_values->Append(0);

			m_node = m_lights->m_list;

			while ((m_node) && (m_node->m_object))
			{
				m_light = (CLight*)m_node->m_object;

				m_constantBuffer->m_values->Append(&m_light->m_position);
				m_constantBuffer->m_values->Append(1.0f);
				m_constantBuffer->m_values->Append(&m_light->m_direction);
				m_constantBuffer->m_values->Append(1.0f);
				m_constantBuffer->m_values->Append(&m_light->m_color);
				m_constantBuffer->m_values->Append(m_light->m_radius);

				m_node = m_node->m_next;
			}
		}
	}
}

/*
*/
CMap::~CMap()
{
	SAFE_DELETE_ARRAY(m_overheadShaders);
	SAFE_DELETE_ARRAY(m_shaders);

	if (m_terrain)
	{
		SAFE_DELETE(m_terrain);
	}

	SAFE_DELETE(m_visibility);
	SAFE_DELETE(m_lights);

	SAFE_DELETE_ARRAY(m_entity);
	SAFE_DELETE_ARRAY(m_shaderMaterial);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);

	m_node = m_vertexBuffers->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_vertexBuffer = (CVertexBuffer*)m_node->m_object;

		SAFE_DELETE(m_vertexBuffer);

		m_node = m_vertexBuffers->Delete(m_node);
	}

	SAFE_DELETE(m_vertexBuffers);

	SAFE_DELETE(m_filename);
	SAFE_DELETE(m_name);
}

/*
*/
void CMap::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_node = m_vertexBuffers->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_vertexBuffer = (CVertexBuffer*)m_node->m_object;

		m_vertexBuffer->SetCommandList(commandList);
		m_vertexBuffer->Record();

		m_node = m_node->m_next;
	}
}

/*
*/
void CMap::Record()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	m_commandList->RSSetViewports(1, m_graphicsAdapter->GetViewport());

	m_commandList->RSSetScissorRects(1, m_graphicsAdapter->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, m_graphicsAdapter->GetRenderTarget(), false, m_graphicsAdapter->GetDepthBuffer());

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_node = m_vertexBuffers->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_vertexBuffer = (CVertexBuffer*)m_node->m_object;

		m_shader = &m_shaders[m_vertexBuffer->m_material];

		m_shader->UpdateConstantBuffers();

		m_shader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_commandList->SetGraphicsRootSignature(m_shader->GetRootSignature());

		m_commandList->SetPipelineState(m_shader->GetPipelineState());

		m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

		m_vertexBuffer->SetCommandList(m_commandList);
		m_vertexBuffer->Record();

		m_node = m_node->m_next;
	}

	m_graphicsAdapter->BundleCommandList(1, m_commandList);
}

/*
*/
void CMap::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, overhead->GetViewport());

	m_commandList->RSSetScissorRects(1, overhead->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, overhead->GetRenderTarget(), false, overhead->GetDepthBuffer());

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_node = m_vertexBuffers->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_vertexBuffer = (CVertexBuffer*)m_node->m_object;

		m_shader = &m_overheadShaders[m_vertexBuffer->m_material];

		m_shader->UpdateConstantBuffers();

		m_shader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_commandList->SetGraphicsRootSignature(m_shader->GetRootSignature());

		m_commandList->SetPipelineState(m_shader->GetPipelineState());

		m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

		m_vertexBuffer->SetCommandList(m_commandList);
		m_vertexBuffer->Record();

		m_node = m_node->m_next;
	}
}

/*
*/
void CMap::SetCurrentCamera(CCamera* camera)
{
	for (int i = 0; i < m_numberOfMaterials; i++)
	{
		m_constantBuffer = m_shaders[i].GetConstantBuffer(0);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(camera->m_xmworld);
		m_constantBuffer->m_values->Append(camera->m_xmview);
		m_constantBuffer->m_values->Append(camera->m_xmproj);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
		m_constantBuffer->m_values->Append(camera->m_position);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(camera->m_look);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
		m_constantBuffer->m_values->Append(1.0f);
	}
}

/*
*/
void CMap::SetOverheadCamera(CCamera* camera)
{
	for (int i = 0; i < m_numberOfMaterials; i++)
	{
		m_constantBuffer = m_overheadShaders[i].GetConstantBuffer(0);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(camera->m_xmworld);
		m_constantBuffer->m_values->Append(camera->m_xmview);
		m_constantBuffer->m_values->Append(camera->m_xmproj);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
		m_constantBuffer->m_values->Append(camera->m_position);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(camera->m_look);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
		m_constantBuffer->m_values->Append(1.0f);
	}
}