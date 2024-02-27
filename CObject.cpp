#include "CObject.h"

/*
*/
CObject::CObject()
{
	memset(this, 0x00, sizeof(CObject));
}

/*
*/
CObject::CObject(CModelManager* modelManager, CWavefrontManager* wavefrontManager, const char* filename, const char* materialname)
{
	memset(this, 0x00, sizeof(CObject));

	m_limboTimer = new CTimer(5000);

	if (materialname == nullptr)
	{
		m_model = modelManager->Create(filename);

		m_meshs = m_model->m_meshs;
	}
	else
	{
		m_wavefront = wavefrontManager->Create(filename, materialname);

		m_meshs = m_wavefront->m_meshs;
	}

	CLinkListNode<CMesh>* mesh = m_meshs->m_list;

	short materialCount = 0;

	while (mesh->m_object)
	{
		m_vertexBuffers[materialCount] = new CVertexBuffer(mesh->m_object->m_vertexCount, (void*)mesh->m_object->m_vertices);

		mesh = mesh->m_next;
	}
}

/*
*/
CObject::CObject(CVideoDevice* videoDevice, CErrorLog* errorLog, UINT descriptorCount)
{
	memset(this, 0x00, sizeof(CObject));

	m_videoDevice = videoDevice;

	m_errorLog = errorLog;

	m_limboTimer = new CTimer(5000);


	for (UINT i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i] = m_videoDevice->CreateCommandAllocator();
	}

	m_commandList = m_videoDevice->CreateCommandList(m_commandAllocators[0]);

	m_commandList->SetName(L"CObject::CObject");


	m_wvp = new CFloat4x4Buffer(m_videoDevice, m_errorLog, m_commandList);

	m_floats[0] = new CFloatBuffer(m_videoDevice, m_errorLog, m_commandList);

	m_ints[0] = new CIntBuffer(m_videoDevice, m_errorLog, m_commandList);


	m_shaderHeap[0] = new CShaderHeap(m_videoDevice, m_errorLog, descriptorCount);

	m_range = new CDescriptorRange();
}

/*
*/
CObject::CObject(CVideoDevice* videoDevice, CErrorLog* errorLog, CModelManager* modelManager, CTextureManager* textureManager, CShaderManager* shaderManager,
	const char* filename,
	const char* vertexShadername, const char* vertexEntrypoint, const char* vertexVersion,
	const char* pixelShaderName, const char* pixelEntryPoint, const char* pixelVersion,
	bool alphaToCoverageEnable, bool blendEnable, bool useOverhead)
{
	memset(this, 0x00, sizeof(CObject));

	m_videoDevice = videoDevice;

	m_errorLog = errorLog;

	m_name = new CString(filename);

	m_limboTimer = new CTimer(5000);

	m_vertexShader = shaderManager->Create(vertexShadername, vertexEntrypoint, vertexVersion);
	m_pixelShader = shaderManager->Create(pixelShaderName, pixelEntryPoint, pixelVersion);

	CObject::CreateDeviceResources(alphaToCoverageEnable, blendEnable);

	m_useOverhead = useOverhead;

	if (m_useOverhead)
	{
		m_overheadWvp = new CFloat4x4Buffer(m_videoDevice, m_errorLog, m_commandList);
	}

	m_model = modelManager->Create(filename);

	m_meshs = m_model->m_meshs;


	CObject::LoadMeshBuffers(textureManager, m_meshs->m_list);
}

/*
*/
CObject::CObject(CVideoDevice* videoDevice, CErrorLog* errorLog, CWavefrontManager* wavefrontManager, CTextureManager* textureManager, CShaderManager* shaderManager,
	const char* filename, const char* materialname,
	const char* vertexShadername, const char* vertexEntrypoint, const char* vertexVersion,
	const char* pixelShaderName, const char* pixelEntryPoint, const char* pixelVersion,
	bool alphaToCoverageEnable, bool blendEnable, bool useOverhead)
{
	memset(this, 0x00, sizeof(CObject));

	m_videoDevice = videoDevice;

	m_errorLog = errorLog;

	m_name = new CString(filename);

	m_limboTimer = new CTimer(5000);

	m_vertexShader = shaderManager->Create(vertexShadername, vertexEntrypoint, vertexVersion);
	m_pixelShader = shaderManager->Create(pixelShaderName, pixelEntryPoint, pixelVersion);

	CObject::CreateDeviceResources(alphaToCoverageEnable, blendEnable);

	m_useOverhead = useOverhead;

	if (m_useOverhead)
	{
		m_overheadWvp = new CFloat4x4Buffer(m_videoDevice, m_errorLog, m_commandList);
	}

	m_wavefront = wavefrontManager->Create(filename, materialname);

	m_meshs = m_wavefront->m_meshs;

	m_scale.x = 1.0f;
	m_scale.y = 1.0f;
	m_scale.z = 1.0f;

	CObject::LoadMeshBuffers(textureManager, m_meshs->m_list);
}

/*
*/
CObject::~CObject()
{
	m_isRecordable = false;

	delete m_pipelineState;
	delete m_rootSignature;

	for (short materialCount = 0; materialCount < m_meshs->m_count; materialCount++)
	{
		delete m_ints[materialCount];
		delete m_floats[materialCount];
		delete m_shaderHeap[materialCount];
		delete m_vertexBuffers[materialCount];

		if (m_useOverhead)
		{
			delete m_overheadInts[materialCount];
			delete m_overheadFloats[materialCount];
			delete m_overheadHeap[materialCount];
		}
	}

	if (m_useOverhead)
	{
		delete m_overheadWvp;
	}

	delete m_range;
	delete m_wvp;

	m_commandList.Reset();

	for (int i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i].Reset();
	}

	delete m_name;
	delete m_limboTimer;
}

/*
*/
void CObject::Animation(CFrametime* frametime)
{
	// bob = non-updating environment position + amp * sin(freq * time);
	m_bob = m_environmentPosition.y + (0.1250f * (float)sin((float)m_count));

	m_spin += 90.0f * frametime->m_frametime;
	if (m_spin > 359.99f)
	{
		m_spin -= 359.99f;
	}

	float freq = 4.0f;

	m_count += freq * frametime->m_frametime;

	m_position.y = m_bob;

	m_rotation.y = m_spin;
}

/*
*/
void CObject::CreateDeviceResources(bool alphaToCoverageEnable, bool blendEnable)
{
	for (UINT i = 0; i < CVideoDevice::E_BACKBUFFER_COUNT; i++)
	{
		m_commandAllocators[i] = m_videoDevice->CreateCommandAllocator();
	}

	m_commandList = m_videoDevice->CreateCommandList(m_commandAllocators[0]);

	m_commandList->SetName(m_name->GetWText());

	m_commandList->Close();


	m_wvp = new CFloat4x4Buffer(m_videoDevice, m_errorLog, m_commandList);


	m_range = new CDescriptorRange(2);

	m_range->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3);
	m_range->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5);

	m_rootSignature = new CRootSignature(m_videoDevice, m_errorLog,
		D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
		2,
		m_range);

	m_pipelineState = new CPipelineState(m_videoDevice, m_errorLog, CVertex::E_VT_VERTEXNT);

	m_pipelineState->Create(alphaToCoverageEnable, blendEnable, true,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		m_vertexShader, m_pixelShader, nullptr, nullptr, nullptr,
		m_rootSignature->m_signature);
}

/*
*/
void CObject::LoadMeshBuffers(CTextureManager* textureManager, CLinkListNode<CMesh>* mesh)
{
	short materialCount = 0;

	while (mesh->m_object)
	{
		m_vertexBuffers[materialCount] = new CVertexBuffer(m_videoDevice, m_errorLog, m_commandList,
			CVertex::E_VT_VERTEXNT,
			mesh->m_object->m_vertexCount,
			(void*)mesh->m_object->m_vertices);

		m_vertexBuffers[materialCount]->m_commandList->SetName(m_name->GetWText());

		m_floats[materialCount] = new CFloatBuffer(m_videoDevice, m_errorLog, m_commandList);

		m_ints[materialCount] = new CIntBuffer(m_videoDevice, m_errorLog, m_commandList);


		m_shaderHeap[materialCount] = new CShaderHeap(m_videoDevice, m_errorLog, 8);

		m_shaderHeap[materialCount]->SlotResource(0, m_wvp);
		m_shaderHeap[materialCount]->SlotResource(1, m_floats[materialCount]);
		m_shaderHeap[materialCount]->SlotResource(2, m_ints[materialCount]);

		if (mesh->m_object->m_material)
		{
			m_floats[materialCount]->m_values[7] = mesh->m_object->m_material->m_opacity;

			if (mesh->m_object->m_map_Kd)
			{
				mesh->m_object->m_material->m_map_Kd = textureManager->Create(mesh->m_object->m_map_Kd->GetText());

				m_shaderHeap[materialCount]->SlotResource(3, mesh->m_object->m_material->m_map_Kd, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
				m_ints[materialCount]->m_values[0] = 1;
			}

			if (mesh->m_object->m_map_Kb)
			{
				mesh->m_object->m_material->m_map_Kb = textureManager->Create(mesh->m_object->m_map_Kb->GetText());

				m_shaderHeap[materialCount]->SlotResource(4, mesh->m_object->m_material->m_map_Kb, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
				m_ints[materialCount]->m_values[1] = 1;
			}

			if (mesh->m_object->m_map_Ks)
			{
				mesh->m_object->m_material->m_map_Ks = textureManager->Create(mesh->m_object->m_map_Ks->GetText());

				m_shaderHeap[materialCount]->SlotResource(5, mesh->m_object->m_material->m_map_Ks, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
				m_ints[materialCount]->m_values[2] = 1;
			}

			if (mesh->m_object->m_map_Ka)
			{
				mesh->m_object->m_material->m_map_Ka = textureManager->Create(mesh->m_object->m_map_Ka->GetText());

				m_shaderHeap[materialCount]->SlotResource(6, mesh->m_object->m_material->m_map_Ka, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
				m_ints[materialCount]->m_values[3] = 1;
			}

			if (mesh->m_object->m_refl)
			{
				mesh->m_object->m_material->m_refl = textureManager->Create(mesh->m_object->m_refl->GetText());

				m_shaderHeap[materialCount]->SlotResource(7, mesh->m_object->m_material->m_refl, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
				m_ints[materialCount]->m_values[4] = 1;
			}


			if (m_useOverhead)
			{
				m_overheadHeap[materialCount] = new CShaderHeap(m_videoDevice, m_errorLog, 8);

				m_overheadFloats[materialCount] = new CFloatBuffer(m_videoDevice, m_errorLog, m_commandList);

				m_overheadInts[materialCount] = new CIntBuffer(m_videoDevice, m_errorLog, m_commandList);

				m_overheadHeap[materialCount]->SlotResource(0, m_overheadWvp);
				m_overheadHeap[materialCount]->SlotResource(1, m_overheadFloats[materialCount]);
				m_overheadHeap[materialCount]->SlotResource(2, m_overheadInts[materialCount]);

				if (mesh->m_object->m_material)
				{
					m_overheadFloats[materialCount]->m_values[7] = mesh->m_object->m_material->m_opacity;

					if (mesh->m_object->m_map_Kd)
					{
						m_overheadHeap[materialCount]->SlotResource(3, mesh->m_object->m_material->m_map_Kd, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
						m_overheadInts[materialCount]->m_values[0] = 1;
					}

					if (mesh->m_object->m_map_Kb)
					{
						m_overheadHeap[materialCount]->SlotResource(4, mesh->m_object->m_material->m_map_Kb, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
						m_overheadInts[materialCount]->m_values[1] = 1;
					}

					if (mesh->m_object->m_map_Ks)
					{
						m_overheadHeap[materialCount]->SlotResource(5, mesh->m_object->m_material->m_map_Ks, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
						m_overheadInts[materialCount]->m_values[2] = 1;
					}

					if (mesh->m_object->m_map_Ka)
					{
						m_overheadHeap[materialCount]->SlotResource(6, mesh->m_object->m_material->m_map_Ka, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
						m_overheadInts[materialCount]->m_values[3] = 1;
					}

					if (mesh->m_object->m_refl)
					{
						m_overheadHeap[materialCount]->SlotResource(7, mesh->m_object->m_material->m_refl, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
						m_overheadInts[materialCount]->m_values[4] = 1;
					}
				}
			}
		}

		materialCount++;

		mesh = mesh->m_next;
	}
}

/*
*/
void CObject::Record()
{
	m_commandAllocators[m_videoDevice->m_backbufferIndex]->Reset();

	m_commandList->Reset(m_commandAllocators[m_videoDevice->m_backbufferIndex].Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_videoDevice->m_swapChainViewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &m_videoDevice->m_swapChainViewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&m_videoDevice->m_swapChainRenderTargets[m_videoDevice->m_backbufferIndex]->m_handle,
		false,
		&m_videoDevice->m_swapChainDepthBuffers[m_videoDevice->m_backbufferIndex]->m_handle);

	m_commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetGraphicsRootSignature(m_rootSignature->m_signature.Get());

	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());


	m_wvp->UpdateConstantBuffer();


	for (short materialCount = 0; materialCount < m_meshs->m_count; materialCount++)
	{
		m_floats[materialCount]->UpdateConstantBuffer();

		m_ints[materialCount]->UpdateConstantBuffer();


		m_commandList->SetDescriptorHeaps(1, m_shaderHeap[materialCount]->m_heap.GetAddressOf());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap[materialCount]->m_heap->GetGPUDescriptorHandleForHeapStart());


		m_vertexBuffers[materialCount]->Draw();
	}
}

/*
*/
void CObject::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, &overhead->m_viewport->m_viewport);

	m_commandList->RSSetScissorRects(1, &overhead->m_viewport->m_scissorRect);

	m_commandList->OMSetRenderTargets(1,
		&overhead->m_renderTargets[m_videoDevice->m_backbufferIndex]->m_handle,
		false,
		&overhead->m_depthBuffers[m_videoDevice->m_backbufferIndex]->m_handle);


	m_overheadWvp->UpdateConstantBuffer();


	for (short materialCount = 0; materialCount < m_meshs->m_count; materialCount++)
	{
		m_overheadFloats[materialCount]->UpdateConstantBuffer();

		m_overheadInts[materialCount]->UpdateConstantBuffer();

		m_commandList->SetDescriptorHeaps(1, m_overheadHeap[materialCount]->m_heap.GetAddressOf());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadHeap[materialCount]->m_heap->GetGPUDescriptorHandleForHeapStart());

		m_vertexBuffers[materialCount]->Draw();
	}
}

/*
*/
void CObject::SetCurrentCamera(CCamera* camera)
{
	XMStoreFloat4x4(&m_wvp->m_values[0], camera->m_xmworld);
	XMStoreFloat4x4(&m_wvp->m_values[1], camera->m_xmview);
	XMStoreFloat4x4(&m_wvp->m_values[2], camera->m_xmproj);
}

/*
*/
void CObject::SetOverheadCamera(CCamera* camera)
{
	XMStoreFloat4x4(&m_overheadWvp->m_values[0], camera->m_xmworld);
	XMStoreFloat4x4(&m_overheadWvp->m_values[1], camera->m_xmview);
	XMStoreFloat4x4(&m_overheadWvp->m_values[2], camera->m_xmproj);
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
void CObject::SetPosition(CVertex* position)
{
	m_position.x = position->p.x;
	m_position.y = position->p.y;
	m_position.z = position->p.z;

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
void CObject::SetRotation(CVertex* rotation)
{
	m_rotation.x = rotation->p.x;
	m_rotation.y = rotation->p.y;
	m_rotation.z = rotation->p.z;
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
void CObject::Update()
{
	CLinkListNode<CMesh>* mn = m_meshs->m_list;

	short materialCount = 0;

	while (mn->m_object)
	{
		m_floats[materialCount]->m_values[7] = mn->m_object->m_material->m_opacity;

		m_vertexBuffers[materialCount]->m_scale.x = m_scale.x;
		m_vertexBuffers[materialCount]->m_scale.y = m_scale.y;
		m_vertexBuffers[materialCount]->m_scale.z = m_scale.z;

		m_vertexBuffers[materialCount]->m_position.x = m_position.x;
		m_vertexBuffers[materialCount]->m_position.y = m_position.y;
		m_vertexBuffers[materialCount]->m_position.z = m_position.z;

		m_vertexBuffers[materialCount]->m_rotation.x = m_rotation.x;
		m_vertexBuffers[materialCount]->m_rotation.y = m_rotation.y;
		m_vertexBuffers[materialCount]->m_rotation.z = m_rotation.z;

		m_vertexBuffers[materialCount]->UpdateRotation();

		m_vertexBuffers[materialCount]->Update(mn->m_object->m_vertices);

		materialCount++;

		mn = mn->m_next;
	}
}

/*
*/
void CObject::UpdateServer()
{
	CLinkListNode<CMesh>* mn = m_meshs->m_list;

	short materialCount = 0;

	while (mn->m_object)
	{
		m_vertexBuffers[materialCount]->m_scale.x = m_scale.x;
		m_vertexBuffers[materialCount]->m_scale.y = m_scale.y;
		m_vertexBuffers[materialCount]->m_scale.z = m_scale.z;

		m_vertexBuffers[materialCount]->m_position.x = m_position.x;
		m_vertexBuffers[materialCount]->m_position.y = m_position.y;
		m_vertexBuffers[materialCount]->m_position.z = m_position.z;

		m_vertexBuffers[materialCount]->m_rotation.x = m_rotation.x;
		m_vertexBuffers[materialCount]->m_rotation.y = m_rotation.y;
		m_vertexBuffers[materialCount]->m_rotation.z = m_rotation.z;

		m_vertexBuffers[materialCount]->UpdateRotation();

		m_vertexBuffers[materialCount]->UpdateServer(mn->m_object->m_vertices);

		materialCount++;

		mn = mn->m_next;
	}
}