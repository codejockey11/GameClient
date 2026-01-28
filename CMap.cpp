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
	CPipelineState* pipelineState, CPipelineState* pipelineStateCollectable, CPipelineState* pipelineStateTerrain, CSoundManager* soundManager,
	int32_t listGroup, CShadowMap* shadowMap, CConstantBuffer* camera, CConstantBuffer* overheadCamera, const char* name, const char* filename)
{
	memset(this, 0x00, sizeof(CMap));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_soundDevice = soundDevice;

	m_wavefrontManager = wavefrontManager;

	m_textureManager = new CTextureManager(m_graphicsAdapter, m_errorLog, m_local, "textures/misc/black.tga");

	m_pipelineState = pipelineState;

	m_pipelineStateCollectable = pipelineStateCollectable;

	m_pipelineStateTerrain = pipelineStateTerrain;

	m_soundManager = soundManager;

	m_listGroup = listGroup;

	m_shadowMap = shadowMap;

	m_cameraConstantBuffer = camera;

	m_overheadCameraConstantBuffer = overheadCamera;

	m_name = new CString(name);

	m_surfaces = new CList();

	m_statics = new CList();

	m_collectables = new CList();

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(m_name->GetWide());

	m_lightConstantBuffer = new CConstantBuffer();

	m_lightConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 1168);

	m_lightConstantBuffer->CreateStaticResource();

	m_blendConstantBuffer = new CConstantBuffer();

	m_blendConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 128);

	m_blendConstantBuffer->CreateStaticResource();

	m_filename = new CString(filename);

	m_loadMessage = new CString(CString::E_LARGE);
}

/*
*/
void CMap::Load()
{
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

			fread_s(&m_materialNumber, sizeof(int32_t), sizeof(int32_t), 1, m_fMap);

			while (m_materialNumber != -1)
			{
				m_surface = new CSurface(m_graphicsAdapter, m_errorLog);

				m_surface->m_material = m_materialNumber;

				m_surface->Read(m_fMap);

				m_surfaces->Append(m_surface, m_vertexBufferCount);

				m_vertexBufferCount++;

				m_loadMessage->SetText(0, "%i", m_vertexBufferCount);

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

			m_loadMessage->SetText(0, "%s", m_objectName);

			m_object = new CObject();

			m_object->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_objectName, m_wavefront, m_textureManager, m_pipelineStateCollectable,
				m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_lightConstantBuffer, true, 3, m_shadowMap);

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

			m_object->RecordConstantBuffers();
			m_object->UploadConstantBuffers();

			m_collectables->Append(m_object, m_object->m_name->m_text);

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

			m_loadMessage->SetText(0, "%s", m_objectName);

			m_object = new CObject();

			m_object->Constructor(m_graphicsAdapter, m_soundDevice, m_errorLog, m_local, m_objectName, m_wavefront, m_textureManager, m_pipelineStateCollectable,
				m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_lightConstantBuffer, true, 3, m_shadowMap);

			m_object->SetPosition(&m_vec3f);

			m_object->Update();

			m_entity[m_entityCount].GetKeyValue("scale", &m_vec3f);

			m_object->SetScale(&m_vec3f);

			m_entity[m_entityCount].GetKeyValue("direction", &m_vec3f);

			m_object->SetRotation(&m_vec3f);

			m_xmfloat4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			m_object->SetDiffuse(&m_xmfloat4);

			m_object->SetAmbient(&m_ambient);

			m_object->RecordConstantBuffers();
			m_object->UploadConstantBuffers();

			m_statics->Append(m_object, m_object->m_name->m_text);

			break;
		}
		case CEntity::Type::TERRAIN:
		{
			CString* terrainFilename = new CString(m_local->m_installPath->m_text);

			char* compile = nullptr;

			m_entity[m_entityCount].GetKeyValue("compile", &compile);

			terrainFilename->Append("main/");
			terrainFilename->Append(compile);

			m_loadMessage->SetText(0, "%s", compile);

			m_terrain = new CTerrain(m_graphicsAdapter, m_errorLog, m_textureManager, m_pipelineStateTerrain, 1, &m_entity[m_entityCount], m_shadowMap,
				m_cameraConstantBuffer, m_overheadCameraConstantBuffer, m_lightConstantBuffer, &m_ambient, m_name->m_text, terrainFilename->m_text);

			SAFE_DELETE(terrainFilename);

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

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Map");

	m_shader->SetConstantBufferCount(4);
	m_shader->SetTextureCount(11);

	m_overheadShader = new CShader();

	m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, "MapOverhead");

	m_overheadShader->SetConstantBufferCount(4);
	m_overheadShader->SetTextureCount(11);

	m_shaderHeaps = new CShaderHeap*[m_numberOfMaterials];

	m_overheadShaderHeaps = new CShaderHeap*[m_numberOfMaterials];

	m_materialConstantBuffers = new CConstantBuffer[m_numberOfMaterials]();

	for (int32_t i = 0; i < m_numberOfMaterials; i++)
	{
		m_materialConstantBuffers[i].Constructor(m_graphicsAdapter, m_errorLog, 104);

		m_materialConstantBuffers[i].CreateStaticResource();

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

		m_materialConstantBuffers[i].RecordStatic();

		m_materialConstantBuffers[i].UploadStaticResources();

		m_materialConstantBuffers[i].ReleaseStaticCPUResource();

		m_shaderHeaps[i] = m_shader->AllocateHeap();

		m_shaderHeap = m_shaderHeaps[i];

		m_shaderHeap->BindResource(0, m_cameraConstantBuffer->m_gpuBuffer.Get(), &m_cameraConstantBuffer->m_srvDesc);
		m_shaderHeap->BindResource(1, m_materialConstantBuffers[i].m_gpuBuffer.Get(), &m_materialConstantBuffers[i].m_srvDesc);
		m_shaderHeap->BindResource(2, m_lightConstantBuffer->m_gpuBuffer.Get(), &m_lightConstantBuffer->m_srvDesc);
		m_shaderHeap->BindResource(3, m_blendConstantBuffer->m_gpuBuffer.Get(), &m_blendConstantBuffer->m_srvDesc);

		m_shaderHeap->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_texture = m_textureManager->Create(m_shaderMaterial[i].m_map_Kd, 0);

		m_loadMessage->SetText(0, "%s", m_shaderMaterial[i].m_map_Kd);

		m_shaderHeap->BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

		m_overheadShaderHeaps[i] = m_overheadShader->AllocateHeap();

		m_overheadShaderHeap = m_overheadShaderHeaps[i];

		m_overheadShaderHeap->BindResource(0, m_overheadCameraConstantBuffer->m_gpuBuffer.Get(), &m_overheadCameraConstantBuffer->m_srvDesc);
		m_overheadShaderHeap->BindResource(1, m_materialConstantBuffers[i].m_gpuBuffer.Get(), &m_materialConstantBuffers[i].m_srvDesc);
		m_overheadShaderHeap->BindResource(2, m_lightConstantBuffer->m_gpuBuffer.Get(), &m_lightConstantBuffer->m_srvDesc);
		m_overheadShaderHeap->BindResource(3, m_blendConstantBuffer->m_gpuBuffer.Get(), &m_blendConstantBuffer->m_srvDesc);

		m_overheadShaderHeap->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_overheadShaderHeap->BindResource(m_overheadShader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
	}
}

/*
*/
CMap::~CMap()
{
	SAFE_DELETE_ARRAY(m_materialConstantBuffers);
	SAFE_DELETE_ARRAY(m_overheadShaderHeaps);
	SAFE_DELETE_ARRAY(m_shaderHeaps);

	SAFE_DELETE(m_overheadShader);
	SAFE_DELETE(m_shader);

	SAFE_DELETE(m_lightConstantBuffer);
	SAFE_DELETE(m_blendConstantBuffer);

	if (m_terrain)
	{
		SAFE_DELETE(m_terrain);
	}

	m_node = m_statics->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_object = (CObject*)m_node->m_object;

		SAFE_DELETE(m_object);

		m_node = m_statics->Delete(m_node);
	}

	SAFE_DELETE(m_statics);
	
	m_node = m_collectables->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_object = (CObject*)m_node->m_object;

		SAFE_DELETE(m_object);

		m_node = m_collectables->Delete(m_node);
	}
	
	SAFE_DELETE(m_collectables);
	
	SAFE_DELETE(m_lights);

	SAFE_DELETE_ARRAY(m_entity);
	SAFE_DELETE_ARRAY(m_shaderMaterial);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);

	m_node = m_surfaces->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_surface = (CSurface*)m_node->m_object;

		SAFE_DELETE(m_surface);

		m_node = m_surfaces->Delete(m_node);
	}

	SAFE_DELETE(m_surfaces);

	SAFE_DELETE(m_textureManager);
	SAFE_DELETE(m_loadMessage);
	SAFE_DELETE(m_filename);
	SAFE_DELETE(m_name);
}

/*
*/
void CMap::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_node = m_surfaces->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_surface = (CSurface*)m_node->m_object;

		commandList->IASetIndexBuffer(&m_surface->m_indexBuffer->m_view);

		m_surface->m_vertexBuffer->DrawIndexed(commandList, m_surface->m_indexBuffer->m_count);

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

	m_node = m_surfaces->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_surface = (CSurface*)m_node->m_object;

		m_shaderHeap = m_shaderHeaps[m_surface->m_material];

		m_commandList->SetDescriptorHeaps(1, m_shaderHeap->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap->GetBaseDescriptor());

		m_commandList->IASetIndexBuffer(&m_surface->m_indexBuffer->m_view);

		m_surface->m_vertexBuffer->DrawIndexed(m_commandList, m_surface->m_indexBuffer->m_count);

		m_node = m_node->m_next;
	}
}

/*
*/
void CMap::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, overhead->GetViewport());

	m_commandList->RSSetScissorRects(1, overhead->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, overhead->GetRenderTarget(), false, overhead->GetDepthBuffer());

	m_node = m_surfaces->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_surface = (CSurface*)m_node->m_object;

		m_overheadShaderHeap = m_overheadShaderHeaps[m_surface->m_material];

		m_commandList->SetDescriptorHeaps(1, m_overheadShaderHeap->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShaderHeap->GetBaseDescriptor());

		m_commandList->IASetIndexBuffer(&m_surface->m_indexBuffer->m_view);

		m_surface->m_vertexBuffer->DrawIndexed(m_commandList, m_surface->m_indexBuffer->m_count);

		m_node = m_node->m_next;
	}
}

/*
*/
void CMap::RecordConstantBuffers()
{
	m_lightConstantBuffer->RecordStatic();
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