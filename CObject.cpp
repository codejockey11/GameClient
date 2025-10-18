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
	CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager, const char* shadername, bool useOverhead, CShadowMap* shadowMap, D3D12_CULL_MODE cullMode)
{
	memset(this, 0x00, sizeof(CObject));

	m_graphicsAdapter = graphicsAdapter;

	m_soundDevice = soundDevice;

	m_errorLog = errorLog;

	m_local = local;

	m_name = new CString(name);

	m_textureManager = textureManager;

	m_shaderBinaryManager = shaderBinaryManager;

	m_useOverhead = useOverhead;

	m_shadowMap = shadowMap;

	m_cullMode = cullMode;

	CString* shaderSource = new CString(shadername);

	shaderSource->Append(".vs");

	m_vertexShaderBinary = m_shaderBinaryManager->Get(shaderSource->m_text);

	delete shaderSource;

	shaderSource = new CString(shadername);

	shaderSource->Append(".ps");

	m_pixelShaderBinary = m_shaderBinaryManager->Get(shaderSource->m_text);

	delete shaderSource;

	for (UINT i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(m_name->GetWide());

	m_commandList->Close();

	m_materialCount = wavefront->m_materialCount;

	m_shader = new CShader[m_materialCount]();

	m_overheadShader = new CShader[m_materialCount]();

	m_shaderMaterial = new CShaderMaterial[m_materialCount]();

	m_vertexBuffers = new CVertexBuffer[m_materialCount]();

	m_hasmap_Kd = new int[m_materialCount]();
	m_hasmap_Ka = new int[m_materialCount]();
	m_hasmap_Ks = new int[m_materialCount]();
	m_hasmap_Ns = new int[m_materialCount]();
	m_hasmap_d = new int[m_materialCount]();
	m_hasmap_bump = new int[m_materialCount]();

	m_meshs = new CLinkList<CMesh>();

	for (int i = 0; i < m_materialCount; i++)
	{
		CMesh* mesh = new CMesh();

		mesh->SetMaterial(wavefront->m_material[i]);
		
		mesh->SetBuffer((BYTE*)wavefront->m_meshvertex[i], wavefront->m_meshvertexCount[i]);

		m_meshs->Add(mesh, mesh->m_material.m_name);
	}

	m_scale.x = 1.0f;
	m_scale.y = 1.0f;
	m_scale.z = 1.0f;

	CObject::LoadMeshBuffers(textureManager);

	m_isInitialized = true;
}

/*
*/
CObject::~CObject()
{
	delete[] m_hasmap_Kd;
	delete[] m_hasmap_Ka;
	delete[] m_hasmap_Ks;
	delete[] m_hasmap_Ns;
	delete[] m_hasmap_d;
	delete[] m_hasmap_bump;

	delete[] m_vertexBuffers;
	delete[] m_shaderMaterial;
	delete[] m_overheadShader;
	delete[] m_shader;

	m_commandList.Reset();

	for (int i = 0; i < CGraphicsAdapter::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}

	delete m_meshs;
	delete m_name;
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
void CObject::SetSpin(float spin)
{
	m_spin = spin;
}

/*
*/
void CObject::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int i = 0; i < m_materialCount; i++)
	{
		m_vertexBuffers[i].SetCommandList(commandList);
		m_vertexBuffers[i].Draw();
	}
}

/*
*/
void CObject::InitMaterialBuffer()
{
	for (int i = 0; i < m_materialCount; i++)
	{
		/*
		for (int c = 0; c < m_shader[i].m_constantBufferCount; c++)
		{
			m_shader[i].m_constantBuffer[c].SetCommandList(m_commandList);
		}

		for (int c = 0; c < m_overheadShader[i].m_constantBufferCount; c++)
		{
			m_overheadShader[i].m_constantBuffer[c].SetCommandList(m_commandList);
		}
		*/
		CShader* shader = &m_shader[i];

		shader->m_constantBuffer[shader->m_b[1]].Reset();

		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_ambient);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_diffuse);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_emissive);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specular);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_illum);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opacity);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specularExponent);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_hasmap_Kd[i]);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_hasmap_Ka[i]);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_hasmap_Ks[i]);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_hasmap_Ns[i]);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_hasmap_d[i]);
		shader->m_constantBuffer[shader->m_b[1]].m_values->Append(m_hasmap_bump[i]);

		CShader* overheadShader = &m_overheadShader[i];

		overheadShader->m_constantBuffer[overheadShader->m_b[1]].Reset();

		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_ambient);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_diffuse);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_emissive);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specular);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_illum);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opacity);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_opticalDensity);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_shaderMaterial[i].m_specularExponent);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_hasmap_Kd[i]);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_hasmap_Ka[i]);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_hasmap_Ks[i]);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_hasmap_Ns[i]);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_hasmap_d[i]);
		overheadShader->m_constantBuffer[overheadShader->m_b[1]].m_values->Append(m_hasmap_bump[i]);
	}
}

/*
*/
void CObject::LoadMeshBuffers(CTextureManager* textureManager)
{
	CLinkListNode<CMesh>* mesh = m_meshs->m_list;

	while (mesh->m_object)
	{
		m_vertexBuffers[mesh->m_object->m_material.m_number].Constructor(m_graphicsAdapter, m_errorLog,
			mesh->m_object->m_material.m_number,
			VertexType::E_VT_VERTEXNT,
			mesh->m_object->m_vertexCount,
			(void*)mesh->m_object->m_vertices);

		CShader* shader = &m_shader[mesh->m_object->m_material.m_number];

		shader->Constructor(m_graphicsAdapter, m_errorLog, m_name->m_text);

		shader->AllocateConstantBuffers(2);

		shader->CreateConstantBuffer(shader->m_b[0], sizeof(XMFLOAT4X4) * 5, m_commandList);
		shader->CreateConstantBuffer(shader->m_b[1], 104, m_commandList);

		shader->SetTextureCount(4);

		shader->AllocateDescriptorRange(2);

		shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, shader->m_constantBufferCount);
		shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shader->m_textureCount);

		shader->AllocateRootParameter();

		shader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, m_cullMode, m_vertexShaderBinary, m_pixelShaderBinary);

		shader->BindConstantBuffers();

		shader->BindResource(shader->m_t[0], m_shadowMap->m_texture->m_texture.Get(), &m_shadowMap->m_texture->m_srvDesc);

		CShaderMaterial* shaderMaterial = &m_shaderMaterial[mesh->m_object->m_material.m_number];
		
		shaderMaterial->m_ambient = mesh->m_object->m_material.m_ambient;
		shaderMaterial->m_diffuse = mesh->m_object->m_material.m_diffuse;
		shaderMaterial->m_emissive = mesh->m_object->m_material.m_emissive;
		shaderMaterial->m_specular = mesh->m_object->m_material.m_specular;
		shaderMaterial->m_illum = mesh->m_object->m_material.m_illum;
		shaderMaterial->m_opacity = mesh->m_object->m_material.m_opacity;
		shaderMaterial->m_opticalDensity = mesh->m_object->m_material.m_opticalDensity;
		shaderMaterial->m_specularExponent = mesh->m_object->m_material.m_specularExponent;

		if (strlen(mesh->m_object->m_material.m_map_Kd) > 0)
		{
			CTexture* texture = textureManager->Create(mesh->m_object->m_material.m_map_Kd);

			shader->BindResource(shader->m_t[1], texture->m_texture.Get(), &texture->m_srvDesc);

			m_hasmap_Kd[mesh->m_object->m_material.m_number] = 1;
		}

		if (strlen(mesh->m_object->m_material.m_map_d) > 0)
		{
			CTexture* texture = textureManager->Create(mesh->m_object->m_material.m_map_d);

			shader->BindResource(shader->m_t[2], texture->m_texture.Get(), &texture->m_srvDesc);

			m_hasmap_d[mesh->m_object->m_material.m_number] = 1;
		}

		if (strlen(mesh->m_object->m_material.m_map_bump) > 0)
		{
			CTexture* texture = textureManager->Create(mesh->m_object->m_material.m_map_bump);

			shader->BindResource(shader->m_t[3], texture->m_texture.Get(), &texture->m_srvDesc);

			m_hasmap_bump[mesh->m_object->m_material.m_number] = 1;
		}

		if (m_useOverhead)
		{
			CShader* overheadShader = &m_overheadShader[mesh->m_object->m_material.m_number];
			
			overheadShader->Constructor(m_graphicsAdapter, m_errorLog, m_name->m_text);

			overheadShader->AllocateConstantBuffers(2);

			overheadShader->CreateConstantBuffer(overheadShader->m_b[0], sizeof(XMFLOAT4X4) * 5, m_commandList);
			overheadShader->CreateConstantBuffer(overheadShader->m_b[1], 104, m_commandList);

			overheadShader->SetTextureCount(4);

			overheadShader->AllocateDescriptorRange(2);

			overheadShader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, overheadShader->m_constantBufferCount);
			overheadShader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, overheadShader->m_textureCount);

			overheadShader->AllocateRootParameter();

			overheadShader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, m_cullMode, m_vertexShaderBinary, m_pixelShaderBinary);

			overheadShader->BindConstantBuffers();

			overheadShader->BindResource(overheadShader->m_t[0], m_shadowMap->m_texture->m_texture.Get(), &m_shadowMap->m_texture->m_srvDesc);

			if (strlen(mesh->m_object->m_material.m_map_Kd) > 0)
			{
				CTexture* texture = textureManager->Create(mesh->m_object->m_material.m_map_Kd);

				overheadShader->BindResource(overheadShader->m_t[1], texture->m_texture.Get(), &texture->m_srvDesc);
			}

			if (strlen(mesh->m_object->m_material.m_map_d) > 0)
			{
				CTexture* texture = textureManager->Create(mesh->m_object->m_material.m_map_d);

				overheadShader->BindResource(overheadShader->m_t[2], texture->m_texture.Get(), &texture->m_srvDesc);
			}

			if (strlen(mesh->m_object->m_material.m_map_bump) > 0)
			{
				CTexture* texture = textureManager->Create(mesh->m_object->m_material.m_map_bump);

				overheadShader->BindResource(overheadShader->m_t[3], texture->m_texture.Get(), &texture->m_srvDesc);
			}
		}

		mesh = mesh->m_next;
	}
}

/*
*/
void CObject::Record()
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

	for (int i = 0; i < m_meshs->m_count; i++)
	{
		m_shader[i].UpdateConstantBuffers();

		m_commandList->SetGraphicsRootSignature(m_shader[i].m_rootSignature->m_signature.Get());

		m_commandList->SetPipelineState(m_shader[i].m_pipelineState->m_pipelineState.Get());

		m_commandList->SetDescriptorHeaps(1, m_shader[i].m_heap->m_heap.GetAddressOf());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shader[i].m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

		m_vertexBuffers[i].SetCommandList(m_commandList);
		m_vertexBuffers[i].Draw();
	}
}

/*
*/
void CObject::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, &overhead->m_viewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &overhead->m_viewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&overhead->m_renderTargets[m_graphicsAdapter->m_backbufferIndex]->m_handle,
		false,
		&overhead->m_depthBuffers[m_graphicsAdapter->m_backbufferIndex]->m_handle);

	for (int i = 0; i < m_meshs->m_count; i++)
	{
		m_overheadShader[i].UpdateConstantBuffers();

		m_commandList->SetDescriptorHeaps(1, m_overheadShader[i].m_heap->m_heap.GetAddressOf());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShader[i].m_heap->m_heap->GetGPUDescriptorHandleForHeapStart());

		m_vertexBuffers[i].SetCommandList(m_commandList);
		m_vertexBuffers[i].Draw();
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
void CObject::SetCurrentCamera(CCamera* camera)
{
	for (int i = 0; i < m_meshs->m_count; i++)
	{
		CShader* shader = &m_shader[i];
		
		shader->m_constantBuffer[shader->m_b[0]].Reset();

		shader->m_constantBuffer[shader->m_b[0]].m_values->Append(camera->m_xmworld);
		shader->m_constantBuffer[shader->m_b[0]].m_values->Append(camera->m_xmview);
		shader->m_constantBuffer[shader->m_b[0]].m_values->Append(camera->m_xmproj);
		shader->m_constantBuffer[shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
		shader->m_constantBuffer[shader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	}
}

/*
*/
void CObject::SetDiffuse(CVec3f* diffuse)
{
	CLinkListNode<CMesh>* mesh = m_meshs->m_list;

	while (mesh->m_object)
	{
		CShaderMaterial* shaderMaterial = &m_shaderMaterial[mesh->m_object->m_material.m_number];

		shaderMaterial->m_diffuse.x = diffuse->m_p.x;
		shaderMaterial->m_diffuse.y = diffuse->m_p.y;
		shaderMaterial->m_diffuse.z = diffuse->m_p.z;
		shaderMaterial->m_diffuse.w = 1.0f;

		mesh = mesh->m_next;
	}
}

/*
*/
void CObject::SetDiffuse(XMFLOAT4* diffuse)
{
	CLinkListNode<CMesh>* mesh = m_meshs->m_list;

	while (mesh->m_object)
	{
		m_shaderMaterial[mesh->m_object->m_material.m_number].m_diffuse = *diffuse;

		mesh = mesh->m_next;
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
void CObject::SetOverheadCamera(CCamera* camera)
{
	for (int i = 0; i < m_meshs->m_count; i++)
	{
		CShader* overheadShader = &m_overheadShader[i];
		
		overheadShader->m_constantBuffer[overheadShader->m_b[0]].Reset();

		overheadShader->m_constantBuffer[overheadShader->m_b[0]].m_values->Append(camera->m_xmworld);
		overheadShader->m_constantBuffer[overheadShader->m_b[0]].m_values->Append(camera->m_xmview);
		overheadShader->m_constantBuffer[overheadShader->m_b[0]].m_values->Append(camera->m_xmproj);
		overheadShader->m_constantBuffer[overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_view);
		overheadShader->m_constantBuffer[overheadShader->m_b[0]].m_values->Append(m_shadowMap->m_light.m_proj);
	}
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
void CObject::Update()
{
	CLinkListNode<CMesh>* mesh = m_meshs->m_list;

	while (mesh->m_object)
	{
		CVertexBuffer* vertexBuffers = &m_vertexBuffers[mesh->m_object->m_material.m_number];

		vertexBuffers->m_scale.x = m_scale.x;
		vertexBuffers->m_scale.y = m_scale.y;
		vertexBuffers->m_scale.z = m_scale.z;

		vertexBuffers->m_position.x = m_position.x;
		vertexBuffers->m_position.y = m_position.y;
		vertexBuffers->m_position.z = m_position.z;

		vertexBuffers->m_rotation.x = m_rotation.x;
		vertexBuffers->m_rotation.y = m_rotation.y;
		vertexBuffers->m_rotation.z = m_rotation.z;

		vertexBuffers->UpdateRotation();

		vertexBuffers->Update(mesh->m_object->m_vertices);

		mesh = mesh->m_next;
	}
}