#include "CObject.h"

/*
*/
CObject::CObject()
{
	memset(this, 0x00, sizeof(CObject));
}

/*
*/
void CObject::Constructor(CGraphicsAdapter* graphicsAdapter, CSoundDevice* soundDevice, CErrorLog* errorLog, CLocal* local, const char* name, CWavefront* wavefront,
	CTextureManager* textureManager, CPipelineState* pipelineState, CConstantBuffer* camera, CConstantBuffer* overheadCamera, CConstantBuffer* light,
	bool useOverhead, int32_t listGroup, CShadowMap* shadowMap)
{
	memset(this, 0x00, sizeof(CObject));

	m_graphicsAdapter = graphicsAdapter;

	m_soundDevice = soundDevice;

	m_errorLog = errorLog;

	m_local = local;

	m_name = new CString(name);

	m_textureManager = textureManager;

	m_pipelineState = pipelineState;

	m_cameraConstantBuffer = camera;

	m_overheadCameraConstantBuffer = overheadCamera;

	m_lightConstantBuffer = light;

	m_useOverhead = useOverhead;

	m_listGroup = listGroup;

	m_shadowMap = shadowMap;

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(m_name->GetWide());

	m_blendConstantBuffer = new CConstantBuffer();

	m_blendConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 128);

	m_blendConstantBuffer->CreateStaticResource();

	m_materialCount = wavefront->m_materialCount;

	m_materialConstantBuffers = new CConstantBuffer[m_materialCount]();

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, m_name->m_text);

	m_shader->SetConstantBufferCount(4);
	m_shader->SetTextureCount(11);

	m_overheadShader = new CShader();

	m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, m_name->m_text);

	m_overheadShader->SetConstantBufferCount(4);
	m_overheadShader->SetTextureCount(11);

	m_shaderMaterials = new CShaderMaterial[m_materialCount]();

	m_vertexBuffers = new CVertexBuffer[m_materialCount]();

	m_shaderHeaps = new CShaderHeap * [m_materialCount];

	m_overheadShaderHeaps = new CShaderHeap * [m_materialCount];

	m_scale.x = 1.0f;
	m_scale.y = 1.0f;
	m_scale.z = 1.0f;

	m_meshs = new CList();

	for (int32_t i = 0; i < m_materialCount; i++)
	{
		m_mesh = new CMesh();

		m_mesh->SetMaterial(&wavefront->m_materials[i]);

		m_mesh->m_material.m_number = i;

		m_mesh->SetBuffer((BYTE*)wavefront->m_meshvertex[i], wavefront->m_meshvertexCount[i]);

		m_meshs->Add(m_mesh, m_mesh->m_material.m_name);
	}

	CObject::LoadMeshBuffers();

	m_graphicsAdapter->WaitForGPU();

	m_isInitialized = true;
	
	m_isVisible = true;
}

/*
*/
CObject::~CObject()
{
	if (m_meshs)
	{
		m_node = m_meshs->m_list;

		while ((m_node) && (m_node->m_object))
		{
			m_mesh = (CMesh*)m_node->m_object;

			SAFE_DELETE(m_mesh);

			m_node = m_meshs->Delete(m_node);
		}
	}

	SAFE_DELETE(m_meshs);

	SAFE_DELETE_ARRAY(m_overheadShaderHeaps);
	SAFE_DELETE_ARRAY(m_shaderHeaps);
	SAFE_DELETE_ARRAY(m_vertexBuffers);
	SAFE_DELETE_ARRAY(m_shaderMaterials);
	
	SAFE_DELETE(m_overheadShader);
	SAFE_DELETE(m_shader);

	SAFE_DELETE_ARRAY(m_materialConstantBuffers);

	SAFE_DELETE(m_blendConstantBuffer);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
	SAFE_DELETE(m_name);
}

/*
*/
void CObject::AddSound(CWavLoader* wavLoader, bool loop)
{
	if (m_soundCount == 2)
	{
		return;
	}

	m_sounds[m_soundCount] = new CSound3D(m_errorLog, m_soundDevice, wavLoader, loop);

	m_soundCount++;
}

/*
*/
void CObject::Animation(CFrametime* frametime)
{
	m_wave = m_environmentPosition.y + (m_amplitude * (float)sin((float)m_count));

	m_degree += m_spin * frametime->m_frametime;

	if (m_degree > 359.99f)
	{
		m_degree -= 359.99f;
	}

	m_count += m_frequency * frametime->m_frametime;

	m_position.y = m_wave;

	m_rotation.y = m_degree;
}

/*
*/
void CObject::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int32_t i = 0; i < m_materialCount; i++)
	{
		m_vertexBuffers[i].Record(commandList);
	}
}

/*
*/
void CObject::LoadMeshBuffers()
{
	m_node = m_meshs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_vertexBuffers[m_mesh->m_material.m_number].Constructor(m_graphicsAdapter, m_errorLog,
			m_mesh->m_material.m_number,
			CPipelineState::VertexType::E_VT_VERTEXNT,
			m_mesh->m_vertexCount,
			(void*)m_mesh->m_vertices);

		m_vertexBuffers[m_mesh->m_material.m_number].CreateDynamicResource();

		m_vertexBuffers[m_mesh->m_material.m_number].LoadDynamicBuffer();

		m_shaderMaterials[m_mesh->m_material.m_number].SetShaderMaterial(&m_mesh->m_material);

		m_materialConstantBuffers[m_mesh->m_material.m_number].Constructor(m_graphicsAdapter, m_errorLog, 104);

		m_materialConstantBuffers[m_mesh->m_material.m_number].CreateStaticResource();

		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_ambient);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_diffuse);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_emissive);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_specular);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_illum);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_opacity);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_opticalDensity);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_specularExponent);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_hasmap_Kd);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_hasmap_Ka);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_hasmap_Ks);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_hasmap_Ns);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_hasmap_d);
		m_materialConstantBuffers[m_mesh->m_material.m_number].m_values->Append(m_shaderMaterials[m_mesh->m_material.m_number].m_hasmap_bump);

		m_shaderHeaps[m_mesh->m_material.m_number] = m_shader->AllocateHeap();

		m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(0, m_cameraConstantBuffer->m_gpuBuffer.Get(), &m_cameraConstantBuffer->m_srvDesc);
		m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(1, m_materialConstantBuffers[m_mesh->m_material.m_number].m_gpuBuffer.Get(), &m_materialConstantBuffers[m_mesh->m_material.m_number].m_srvDesc);
		//m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(2, m_lightConstantBuffer->m_gpuBuffer.Get(), &m_lightConstantBuffer->m_srvDesc);
		m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(3, m_blendConstantBuffer->m_gpuBuffer.Get(), &m_blendConstantBuffer->m_srvDesc);

		//m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		if (strlen(m_mesh->m_material.m_map_Kd) > 0)
		{
			m_texture = m_textureManager->Create(m_mesh->m_material.m_map_Kd, 0);

			m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
		}

		if (strlen(m_mesh->m_material.m_map_d) > 0)
		{
			m_texture = m_textureManager->Create(m_mesh->m_material.m_map_d, 0);

			m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(m_shader->m_t[2], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
		}

		if (strlen(m_mesh->m_material.m_map_bump) > 0)
		{
			m_texture = m_textureManager->Create(m_mesh->m_material.m_map_bump, 0);

			m_shaderHeaps[m_mesh->m_material.m_number]->BindResource(m_shader->m_t[3], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
		}

		if (m_useOverhead)
		{
			m_overheadShaderHeaps[m_mesh->m_material.m_number] = m_overheadShader->AllocateHeap();

			m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(0, m_overheadCameraConstantBuffer->m_gpuBuffer.Get(), &m_overheadCameraConstantBuffer->m_srvDesc);
			m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(1, m_materialConstantBuffers[m_mesh->m_material.m_number].m_gpuBuffer.Get(), &m_materialConstantBuffers[m_mesh->m_material.m_number].m_srvDesc);
			//m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(2, m_lightConstantBuffer->m_gpuBuffer.Get(), &m_lightConstantBuffer->m_srvDesc);
			m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(3, m_blendConstantBuffer->m_gpuBuffer.Get(), &m_blendConstantBuffer->m_srvDesc);

			//m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(m_overheadShader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

			if (strlen(m_mesh->m_material.m_map_Kd) > 0)
			{
				m_texture = m_textureManager->Create(m_mesh->m_material.m_map_Kd, 0);

				m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(m_overheadShader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
			}

			if (strlen(m_mesh->m_material.m_map_d) > 0)
			{
				m_texture = m_textureManager->Create(m_mesh->m_material.m_map_d, 0);

				m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(m_overheadShader->m_t[2], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
			}

			if (strlen(m_mesh->m_material.m_map_bump) > 0)
			{
				m_texture = m_textureManager->Create(m_mesh->m_material.m_map_bump, 0);

				m_overheadShaderHeaps[m_mesh->m_material.m_number]->BindResource(m_overheadShader->m_t[3], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
			}
		}

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::Record()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	m_commandList->RSSetViewports(1, m_graphicsAdapter->GetViewport());

	m_commandList->RSSetScissorRects(1, m_graphicsAdapter->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, m_graphicsAdapter->GetRenderTarget(), false, m_graphicsAdapter->GetDepthBuffer());

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());

	m_node = m_meshs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_commandList->SetDescriptorHeaps(1, m_shaderHeaps[m_mesh->m_material.m_number]->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeaps[m_mesh->m_material.m_number]->GetBaseDescriptor());

		m_vertexBuffers[m_mesh->m_material.m_number].Record(m_commandList);

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, overhead->GetViewport());

	m_commandList->RSSetScissorRects(1, overhead->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, overhead->GetRenderTarget(), false, overhead->GetDepthBuffer());

	m_node = m_meshs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_commandList->SetDescriptorHeaps(1, m_overheadShaderHeaps[m_mesh->m_material.m_number]->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShaderHeaps[m_mesh->m_material.m_number]->GetBaseDescriptor());

		m_vertexBuffers[m_mesh->m_material.m_number].Record(m_commandList);

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::RecordConstantBuffers()
{
	for (int32_t i = 0; i < m_materialCount; i++)
	{
		if (m_shaderMaterials[i].m_needsUpload)
		{
			m_materialConstantBuffers[i].Reset();

			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_ambient);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_diffuse);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_emissive);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_specular);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_illum);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_opacity);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_opticalDensity);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_specularExponent);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_hasmap_Kd);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_hasmap_Ka);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_hasmap_Ks);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_hasmap_Ns);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_hasmap_d);
			m_materialConstantBuffers[i].m_values->Append(m_shaderMaterials[i].m_hasmap_bump);

			m_materialConstantBuffers[i].RecordStatic();

			m_shaderMaterials[i].m_needsUpload = false;
		}
	}
}

/*
*/
void CObject::SetAmbient(CVec3f* ambient)
{
	m_node = m_meshs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_shaderMaterials[m_mesh->m_material.m_number].SetKa(ambient->m_p.x, ambient->m_p.y, ambient->m_p.z, 1.0f);

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::SetAmplitude(float amplitude)
{
	m_amplitude = amplitude;
}

/*
*/
void CObject::SetDiffuse(CVec3f* diffuse)
{
	m_node = m_meshs->m_list;

	while (m_node->m_object)
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_shaderMaterials[m_mesh->m_material.m_number].SetKd(diffuse->m_p.x, diffuse->m_p.y, diffuse->m_p.z, 1.0f);

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::SetDiffuse(XMFLOAT4* diffuse)
{
	m_node = m_meshs->m_list;

	while (m_node->m_object)
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_shaderMaterials[m_mesh->m_material.m_number].SetKd(diffuse->x, diffuse->y, diffuse->z, 1.0f);

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::SetFrequency(float frequency)
{
	m_frequency = frequency;
}

/*
*/
void CObject::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;

	m_environmentPosition = m_position;
}

/*
*/
void CObject::SetPosition(CVec3f* position)
{
	m_position.x = position->m_p.x;
	m_position.y = position->m_p.y;
	m_position.z = position->m_p.z;

	m_environmentPosition = m_position;
}

/*
*/
void CObject::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

/*
*/
void CObject::SetRotation(CVec3f* rotation)
{
	m_rotation.x = rotation->m_p.x;
	m_rotation.y = rotation->m_p.y;
	m_rotation.z = rotation->m_p.z;
}

/*
*/
void CObject::SetScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
}

/*
*/
void CObject::SetScale(CVec3f* scale)
{
	m_scale.x = scale->m_p.x;
	m_scale.y = scale->m_p.y;
	m_scale.z = scale->m_p.z;
}

/*
*/
void CObject::SetSpin(float spin)
{
	m_spin = spin;
}

/*
*/
void CObject::Update()
{
	m_node = m_meshs->m_list;

	while ((m_node) && (m_node->m_object))
	{
		m_mesh = (CMesh*)m_node->m_object;

		m_vertexBuffers[m_mesh->m_material.m_number].m_scale.x = m_scale.x;
		m_vertexBuffers[m_mesh->m_material.m_number].m_scale.y = m_scale.y;
		m_vertexBuffers[m_mesh->m_material.m_number].m_scale.z = m_scale.z;

		m_vertexBuffers[m_mesh->m_material.m_number].m_position.x = m_position.x;
		m_vertexBuffers[m_mesh->m_material.m_number].m_position.y = m_position.y;
		m_vertexBuffers[m_mesh->m_material.m_number].m_position.z = m_position.z;

		m_vertexBuffers[m_mesh->m_material.m_number].m_rotation.x = m_rotation.x;
		m_vertexBuffers[m_mesh->m_material.m_number].m_rotation.y = m_rotation.y;
		m_vertexBuffers[m_mesh->m_material.m_number].m_rotation.z = m_rotation.z;

		m_vertexBuffers[m_mesh->m_material.m_number].UpdateRotation();

		m_vertexBuffers[m_mesh->m_material.m_number].UpdateDynamicBuffer();

		m_node = m_node->m_next;
	}
}

/*
*/
void CObject::UploadConstantBuffers()
{
	for (int32_t i = 0; i < m_materialCount; i++)
	{
		m_materialConstantBuffers[i].m_commandList->Close();

		m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_materialConstantBuffers[i].m_commandList.GetAddressOf());

		m_graphicsAdapter->WaitForGPU();
	}
}