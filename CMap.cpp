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

	fread_s(&m_numberOfMaterials, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);

	m_shaderMaterial = new CShaderMaterial[m_numberOfMaterials]();

	fread_s(m_shaderMaterial, sizeof(CShaderMaterial) * m_numberOfMaterials, sizeof(CShaderMaterial), m_numberOfMaterials, m_fMap);

	fread_s(&m_maxEntityCount, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);

	m_entity = new CEntity[m_maxEntityCount]();

	while (!feof(m_fMap))
	{
		fread_s(&m_entity[m_entityCount].m_number, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);
		fread_s(&m_entity[m_entityCount].m_type, sizeof(unsigned char), sizeof(unsigned char), 1, m_fMap);

		fread_s(&m_keyValueCount, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);

		m_entity[m_entityCount].Constructor(m_keyValueCount);

		for (int32_t i = 0; i < m_keyValueCount; i++)
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

			fread_s(&m_materialNumber, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);

			while (m_materialNumber != -1)
			{
				fread_s(&m_verticesCount, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);

				m_vertices = new CVertexNT[m_verticesCount]();

				fread_s(m_vertices, sizeof(CVertexNT) * m_verticesCount, sizeof(CVertexNT), m_verticesCount, m_fMap);

				m_vertexBuffer = new CVertexBuffer();

				m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_materialNumber, VertexType::E_VT_VERTEXNT, m_verticesCount, (void*)m_vertices);

				m_vertexBuffer->SetCommandList(m_commandList);

				m_vertexBuffers->Append(m_vertexBuffer, m_vertexBufferCount);

				SAFE_DELETE_ARRAY(m_vertices);

				m_vertexBufferCount++;

				fread_s(&m_materialNumber, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);
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
		for (int32_t pz = 0; pz < m_visibility->m_sector->m_gridDepth; pz++)
		{
			for (int32_t px = 0; px < m_visibility->m_sector->m_gridWidth; px++)
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

	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog, VertexType::E_VT_VERTEXNT, D3D12_CULL_MODE::D3D12_CULL_MODE_BACK);

	m_pipelineState->AllocateSignature(4, 11);

	m_pipelineState->SetPixelShader(m_ps);
	m_pipelineState->SetVertexShader(m_vs);

	m_pipelineState->Create(false, true, true, D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_cameraConstantBuffer = new CConstantBuffer();

	m_cameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 384);

	m_overheadCameraConstantBuffer = new CConstantBuffer();

	m_overheadCameraConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 384);

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

	m_materialConstantBuffers = new CConstantBuffer[m_numberOfMaterials]();

	m_shaders = new CShader[m_numberOfMaterials]();

	m_overheadShaders = new CShader[m_numberOfMaterials]();

	for (int32_t i = 0; i < m_numberOfMaterials; i++)
	{
		m_materialConstantBuffers[i].Constructor(m_graphicsAdapter, m_errorLog, m_commandList, 104);

		m_materialConstantBuffers[i].Reset();

		m_materialConstantBuffers[i].m_values->Append(&m_ambient);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_diffuse);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_emissive);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_specular);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_illum);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_opacity);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		m_materialConstantBuffers[i].m_values->Append(m_shaderMaterial[i].m_specularExponent);
		m_materialConstantBuffers[i].m_values->Append(1);

		m_shader = &m_shaders[i];

		m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Map");

		m_shader->SetConstantBufferCount(4);
		m_shader->SetTextureCount(11);

		m_shader->AllocateHeap();

		m_shader->BindResource(0, m_cameraConstantBuffer->m_buffer.Get(), &m_cameraConstantBuffer->m_srvDesc);
		m_shader->BindResource(1, m_materialConstantBuffers[i].m_buffer.Get(), &m_materialConstantBuffers[i].m_srvDesc);
		m_shader->BindResource(2, m_lightConstantBuffer->m_buffer.Get(), &m_lightConstantBuffer->m_srvDesc);
		m_shader->BindResource(3, m_blendConstantBuffer->m_buffer.Get(), &m_blendConstantBuffer->m_srvDesc);

		m_shader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_texture = m_textureManager->Create(m_shaderMaterial[i].m_map_Kd);

		m_shader->BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

		m_overheadShader = &m_overheadShaders[i];

		m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, "MapOverhead");

		m_overheadShader->SetConstantBufferCount(4);
		m_overheadShader->SetTextureCount(11);

		m_overheadShader->AllocateHeap();

		m_overheadShader->BindResource(0, m_overheadCameraConstantBuffer->m_buffer.Get(), &m_overheadCameraConstantBuffer->m_srvDesc);
		m_overheadShader->BindResource(1, m_materialConstantBuffers[i].m_buffer.Get(), &m_materialConstantBuffers[i].m_srvDesc);
		m_overheadShader->BindResource(2, m_lightConstantBuffer->m_buffer.Get(), &m_lightConstantBuffer->m_srvDesc);
		m_overheadShader->BindResource(3, m_blendConstantBuffer->m_buffer.Get(), &m_blendConstantBuffer->m_srvDesc);

		m_overheadShader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_overheadShader->BindResource(m_overheadShader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
	}
}

/*
*/
CMap::~CMap()
{
	SAFE_DELETE_ARRAY(m_overheadShaders);
	SAFE_DELETE_ARRAY(m_shaders);
	SAFE_DELETE_ARRAY(m_materialConstantBuffers);

	SAFE_DELETE(m_lightConstantBuffer);
	SAFE_DELETE(m_blendConstantBuffer);
	SAFE_DELETE(m_overheadCameraConstantBuffer);
	SAFE_DELETE(m_cameraConstantBuffer);
	SAFE_DELETE(m_pipelineState);

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

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());

	m_cameraConstantBuffer->UpdateBuffer();

	m_lightConstantBuffer->UpdateBuffer();

	m_blendConstantBuffer->UpdateBuffer();

	m_node = m_vertexBuffers->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_vertexBuffer = (CVertexBuffer*)m_node->m_object;

		m_shader = &m_shaders[m_vertexBuffer->m_material];

		m_materialConstantBuffers[m_vertexBuffer->m_material].UpdateBuffer();

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

	m_overheadCameraConstantBuffer->UpdateBuffer();

	m_node = m_vertexBuffers->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_vertexBuffer = (CVertexBuffer*)m_node->m_object;

		m_shader = &m_overheadShaders[m_vertexBuffer->m_material];

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
void CMap::SetLights(CList* lights)
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
void CMap::SetOverheadCamera(CCamera* camera)
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