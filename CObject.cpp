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

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter);

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->SetName(m_name->GetWide());

	m_commandList->Close();

	m_materialCount = wavefront->m_materialCount;

	m_shaders = new CShader[m_materialCount]();

	m_overheadShaders = new CShader[m_materialCount]();

	m_shaderMaterials = new CShaderMaterial[m_materialCount]();

	m_vertexBuffers = new CVertexBuffer[m_materialCount]();

	m_hasmap_Kd = new int[m_materialCount]();
	m_hasmap_Ka = new int[m_materialCount]();
	m_hasmap_Ks = new int[m_materialCount]();
	m_hasmap_Ns = new int[m_materialCount]();
	m_hasmap_d = new int[m_materialCount]();
	m_hasmap_bump = new int[m_materialCount]();

	m_scale.x = 1.0f;
	m_scale.y = 1.0f;
	m_scale.z = 1.0f;

	m_meshs = new CLinkList<CMesh>();

	for (int i = 0; i < m_materialCount; i++)
	{
		m_mesh = new CMesh();

		m_mesh->SetMaterial(wavefront->m_materials[i]);

		m_mesh->SetBuffer((BYTE*)wavefront->m_meshvertex[i], wavefront->m_meshvertexCount[i]);

		m_meshs->Add(m_mesh, m_mesh->m_material.m_name);
	}

	CObject::LoadMeshBuffers(textureManager);

	m_isInitialized = true;
}

/*
*/
CObject::~CObject()
{
	delete m_meshs;

	delete[] m_hasmap_Kd;
	delete[] m_hasmap_Ka;
	delete[] m_hasmap_Ks;
	delete[] m_hasmap_Ns;
	delete[] m_hasmap_d;
	delete[] m_hasmap_bump;

	delete[] m_vertexBuffers;
	delete[] m_shaderMaterials;
	delete[] m_overheadShaders;
	delete[] m_shaders;

	m_commandList.Reset();

	delete m_commandAllocator;

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
void CObject::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (int i = 0; i < m_materialCount; i++)
	{
		m_vertexBuffer = &m_vertexBuffers[i];

		m_vertexBuffer->SetCommandList(commandList);
		m_vertexBuffer->Record();
	}
}

/*
*/
void CObject::InitMaterialBuffer()
{
	for (int i = 0; i < m_materialCount; i++)
	{
		m_constantBuffer = m_shaders[i].GetConstantBuffer(1);

		m_shaderMaterial = &m_shaderMaterials[i];

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(m_shaderMaterial->m_ambient);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_diffuse);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_emissive);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_specular);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_illum);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_opacity);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_opticalDensity);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_specularExponent);
		m_constantBuffer->m_values->Append(m_hasmap_Kd[i]);
		m_constantBuffer->m_values->Append(m_hasmap_Ka[i]);
		m_constantBuffer->m_values->Append(m_hasmap_Ks[i]);
		m_constantBuffer->m_values->Append(m_hasmap_Ns[i]);
		m_constantBuffer->m_values->Append(m_hasmap_d[i]);
		m_constantBuffer->m_values->Append(m_hasmap_bump[i]);

		m_constantBuffer = m_overheadShaders[i].GetConstantBuffer(1);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(m_shaderMaterial->m_ambient);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_diffuse);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_emissive);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_specular);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_illum);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_opacity);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_opticalDensity);
		m_constantBuffer->m_values->Append(m_shaderMaterial->m_specularExponent);
		m_constantBuffer->m_values->Append(m_hasmap_Kd[i]);
		m_constantBuffer->m_values->Append(m_hasmap_Ka[i]);
		m_constantBuffer->m_values->Append(m_hasmap_Ks[i]);
		m_constantBuffer->m_values->Append(m_hasmap_Ns[i]);
		m_constantBuffer->m_values->Append(m_hasmap_d[i]);
		m_constantBuffer->m_values->Append(m_hasmap_bump[i]);
	}
}

/*
*/
void CObject::LoadMeshBuffers(CTextureManager* textureManager)
{
	m_meshNode = m_meshs->m_list;

	while (m_meshNode->m_object)
	{
		m_vertexBuffers[m_meshNode->m_object->m_material.m_number].Constructor(m_graphicsAdapter, m_errorLog,
			m_meshNode->m_object->m_material.m_number,
			VertexType::E_VT_VERTEXNT,
			m_meshNode->m_object->m_vertexCount,
			(void*)m_meshNode->m_object->m_vertices);

		m_shader = &m_shaders[m_meshNode->m_object->m_material.m_number];

		m_shader->Constructor(m_graphicsAdapter, m_errorLog, m_name->m_text);

		m_shader->AllocateConstantBuffers(4);

		m_shader->SetTextureCount(11);

		m_shader->CreateConstantBuffer(0, 352, m_commandList);
		m_shader->CreateConstantBuffer(1, 104, m_commandList);
		m_shader->CreateConstantBuffer(2, 260, m_commandList);
		m_shader->CreateConstantBuffer(3, 128, m_commandList);

		m_shader->AllocateDescriptorRange(2);

		m_shader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_shader->m_constantBufferCount);
		m_shader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_shader->m_textureCount);

		m_shader->AllocateRootParameter();

		m_shader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, m_cullMode, m_vertexShaderBinary, m_pixelShaderBinary);

		m_shader->BindConstantBuffers();

		m_shaderMaterial = &m_shaderMaterials[m_meshNode->m_object->m_material.m_number];

		m_shaderMaterial->m_ambient = m_meshNode->m_object->m_material.m_ambient;
		m_shaderMaterial->m_diffuse = m_meshNode->m_object->m_material.m_diffuse;
		m_shaderMaterial->m_emissive = m_meshNode->m_object->m_material.m_emissive;
		m_shaderMaterial->m_specular = m_meshNode->m_object->m_material.m_specular;
		m_shaderMaterial->m_illum = m_meshNode->m_object->m_material.m_illum;
		m_shaderMaterial->m_opacity = m_meshNode->m_object->m_material.m_opacity;
		m_shaderMaterial->m_opticalDensity = m_meshNode->m_object->m_material.m_opticalDensity;
		m_shaderMaterial->m_specularExponent = m_meshNode->m_object->m_material.m_specularExponent;

		if (strlen(m_meshNode->m_object->m_material.m_map_Kd) > 0)
		{
			m_texture = textureManager->Create(m_meshNode->m_object->m_material.m_map_Kd);

			m_shader->BindResource(m_shader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

			m_hasmap_Kd[m_meshNode->m_object->m_material.m_number] = 1;
		}

		if (strlen(m_meshNode->m_object->m_material.m_map_d) > 0)
		{
			m_texture = textureManager->Create(m_meshNode->m_object->m_material.m_map_d);

			m_shader->BindResource(m_shader->m_t[2], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

			m_hasmap_d[m_meshNode->m_object->m_material.m_number] = 1;
		}

		if (strlen(m_meshNode->m_object->m_material.m_map_bump) > 0)
		{
			m_texture = textureManager->Create(m_meshNode->m_object->m_material.m_map_bump);

			m_shader->BindResource(m_shader->m_t[3], m_texture->m_texture.Get(), &m_texture->m_srvDesc);

			m_hasmap_bump[m_meshNode->m_object->m_material.m_number] = 1;
		}

		if (m_useOverhead)
		{
			m_overheadShader = &m_overheadShaders[m_meshNode->m_object->m_material.m_number];

			m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, m_name->m_text);

			m_overheadShader->AllocateConstantBuffers(4);

			m_overheadShader->SetTextureCount(11);

			m_overheadShader->CreateConstantBuffer(0, 352, m_commandList);
			m_overheadShader->CreateConstantBuffer(1, 104, m_commandList);
			m_overheadShader->CreateConstantBuffer(2, 260, m_commandList);
			m_overheadShader->CreateConstantBuffer(3, 128, m_commandList);

			m_overheadShader->AllocateDescriptorRange(2);

			m_overheadShader->m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_overheadShader->m_constantBufferCount);
			m_overheadShader->m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_overheadShader->m_textureCount);

			m_overheadShader->AllocateRootParameter();

			m_overheadShader->AllocatePipelineState(true, VertexType::E_VT_VERTEXNT, m_cullMode, m_vertexShaderBinary, m_pixelShaderBinary);

			m_overheadShader->BindConstantBuffers();

			if (strlen(m_meshNode->m_object->m_material.m_map_Kd) > 0)
			{
				m_texture = textureManager->Create(m_meshNode->m_object->m_material.m_map_Kd);

				m_overheadShader->BindResource(m_overheadShader->m_t[1], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
			}

			if (strlen(m_meshNode->m_object->m_material.m_map_d) > 0)
			{
				m_texture = textureManager->Create(m_meshNode->m_object->m_material.m_map_d);

				m_overheadShader->BindResource(m_overheadShader->m_t[2], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
			}

			if (strlen(m_meshNode->m_object->m_material.m_map_bump) > 0)
			{
				m_texture = textureManager->Create(m_meshNode->m_object->m_material.m_map_bump);

				m_overheadShader->BindResource(m_overheadShader->m_t[3], m_texture->m_texture.Get(), &m_texture->m_srvDesc);
			}
		}

		m_meshNode = m_meshNode->m_next;
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

	for (int i = 0; i < m_meshs->m_count; i++)
	{
		m_shader = &m_shaders[i];

		m_shader->UpdateConstantBuffers();

		m_shader->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_commandList->SetGraphicsRootSignature(m_shader->GetRootSignature());

		m_commandList->SetPipelineState(m_shader->GetPipelineState());

		m_commandList->SetDescriptorHeaps(1, m_shader->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shader->GetBaseDescriptor());

		m_vertexBuffer = &m_vertexBuffers[i];

		m_vertexBuffer->SetCommandList(m_commandList);
		m_vertexBuffer->Record();
	}
}

/*
*/
void CObject::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, overhead->GetViewport());

	m_commandList->RSSetScissorRects(1, overhead->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, overhead->GetRenderTarget(), false, overhead->GetDepthBuffer());

	for (int i = 0; i < m_meshs->m_count; i++)
	{
		m_overheadShader = &m_overheadShaders[i];

		m_overheadShader->UpdateConstantBuffers();

		m_overheadShader->BindResource(m_overheadShader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

		m_commandList->SetDescriptorHeaps(1, m_overheadShader->GetDescriptorHeap());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShader->GetBaseDescriptor());

		m_vertexBuffer = &m_vertexBuffers[i];

		m_vertexBuffer->SetCommandList(m_commandList);
		m_vertexBuffer->Record();
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
		m_constantBuffer = m_shaders[i].GetConstantBuffer(0);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(camera->m_xmworld);
		m_constantBuffer->m_values->Append(camera->m_xmview);
		m_constantBuffer->m_values->Append(camera->m_xmproj);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
		m_constantBuffer->m_values->Append(1.0f);
	}
}

/*
*/
void CObject::SetDiffuse(CVec3f* diffuse)
{
	m_meshNode = m_meshs->m_list;

	while (m_meshNode->m_object)
	{
		m_shaderMaterial = &m_shaderMaterials[m_meshNode->m_object->m_material.m_number];

		m_shaderMaterial->m_diffuse.x = diffuse->m_p.x;
		m_shaderMaterial->m_diffuse.y = diffuse->m_p.y;
		m_shaderMaterial->m_diffuse.z = diffuse->m_p.z;
		m_shaderMaterial->m_diffuse.w = 1.0f;

		m_meshNode = m_meshNode->m_next;
	}
}

/*
*/
void CObject::SetDiffuse(XMFLOAT4* diffuse)
{
	m_meshNode = m_meshs->m_list;

	while (m_meshNode->m_object)
	{
		m_shaderMaterials[m_meshNode->m_object->m_material.m_number].m_diffuse = *diffuse;

		m_meshNode = m_meshNode->m_next;
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
void CObject::SetLight(CLinkList<CLight>* lights)
{
	m_lights = lights;

	if (m_lights == nullptr)
	{
		return;
	}

	CConstantBuffer* constantBuffer = m_shader->GetConstantBuffer(2);

	constantBuffer->Reset();

	constantBuffer->m_values->Append(m_lights->m_count);
	constantBuffer->m_values->Append(0);
	constantBuffer->m_values->Append(0);
	constantBuffer->m_values->Append(0);

	m_lightNode = m_lights->m_list;

	while (m_lightNode->m_object)
	{
		constantBuffer->m_values->Append(&m_lightNode->m_object->m_position);
		constantBuffer->m_values->Append(1.0f);
		constantBuffer->m_values->Append(&m_lightNode->m_object->m_color);
		constantBuffer->m_values->Append(m_lightNode->m_object->m_radius);

		m_lightNode = m_lightNode->m_next;
	}

	constantBuffer = m_overheadShader->GetConstantBuffer(2);

	constantBuffer->Reset();

	constantBuffer->m_values->Append(m_lights->m_count);
	constantBuffer->m_values->Append(0);
	constantBuffer->m_values->Append(0);
	constantBuffer->m_values->Append(0);

	m_lightNode = m_lights->m_list;

	while (m_lightNode->m_object)
	{
		constantBuffer->m_values->Append(&m_lightNode->m_object->m_position);
		constantBuffer->m_values->Append(1.0f);
		constantBuffer->m_values->Append(&m_lightNode->m_object->m_color);
		constantBuffer->m_values->Append(m_lightNode->m_object->m_radius);

		m_lightNode = m_lightNode->m_next;
	}
}

/*
*/
void CObject::SetOverheadCamera(CCamera* camera)
{
	for (int i = 0; i < m_meshs->m_count; i++)
	{
		m_constantBuffer = m_overheadShaders[i].GetConstantBuffer(0);

		m_constantBuffer->Reset();

		m_constantBuffer->m_values->Append(camera->m_xmworld);
		m_constantBuffer->m_values->Append(camera->m_xmview);
		m_constantBuffer->m_values->Append(camera->m_xmproj);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_view);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_proj);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_position.m_p);
		m_constantBuffer->m_values->Append(1.0f);
		m_constantBuffer->m_values->Append(m_shadowMap->m_light->m_direction.m_p);
		m_constantBuffer->m_values->Append(1.0f);
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
void CObject::SetSpin(float spin)
{
	m_spin = spin;
}

/*
*/
void CObject::Update()
{
	m_meshNode = m_meshs->m_list;

	while (m_meshNode->m_object)
	{
		m_vertexBuffer = &m_vertexBuffers[m_meshNode->m_object->m_material.m_number];

		m_vertexBuffer->m_scale.x = m_scale.x;
		m_vertexBuffer->m_scale.y = m_scale.y;
		m_vertexBuffer->m_scale.z = m_scale.z;

		m_vertexBuffer->m_position.x = m_position.x;
		m_vertexBuffer->m_position.y = m_position.y;
		m_vertexBuffer->m_position.z = m_position.z;

		m_vertexBuffer->m_rotation.x = m_rotation.x;
		m_vertexBuffer->m_rotation.y = m_rotation.y;
		m_vertexBuffer->m_rotation.z = m_rotation.z;

		m_vertexBuffer->UpdateRotation();

		m_vertexBuffer->Update(m_meshNode->m_object->m_vertices);

		m_meshNode = m_meshNode->m_next;
	}
}