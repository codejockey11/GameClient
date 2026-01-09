#include "CTerrain.h"

/*
*/
CTerrain::CTerrain()
{
	memset(this, 0x00, sizeof(CTerrain));
}

/*
*/
CTerrain::CTerrain(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CTextureManager* textureManager, CPipelineState* pipelineState, int32_t listGroup,
	CEntity* entity, CShadowMap* shadowMap, CConstantBuffer* camera, CConstantBuffer* overheadCamera, CConstantBuffer* light, CVec3f* ambient,
	const char* name, const char* filename)
{
	memset(this, 0x00, sizeof(CTerrain));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_textureManager = textureManager;

	m_pipelineState = pipelineState;

	m_listGroup = listGroup;

	m_entity = entity;

	m_shadowMap = shadowMap;

	m_cameraConstantBuffer = camera;

	m_overheadCameraConstantBuffer = overheadCamera;

	m_lightConstantBuffer = light;

	m_name = new CString(name);

	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(m_name->GetWide());

	m_err = fopen_s(&m_fTerrain, filename, "rb");

	if (m_err)
	{
		m_errorLog->WriteError("CTerrain::CTerrain:Terrain not found:%s\n", filename);

		return;
	}

	m_entity->GetKeyValue("width", &m_width);
	m_entity->GetKeyValue("depth", &m_depth);

	m_vertices = new CVertexNT[m_width * m_depth]();

	fread_s(m_vertices, sizeof(CVertexNT) * m_width * m_depth, sizeof(CVertexNT), (size_t)m_width * m_depth, m_fTerrain);

	fclose(m_fTerrain);

	m_vertexBuffer = new CVertexBuffer();

	m_vertexBuffer->Constructor(m_graphicsAdapter, m_errorLog, NULL, CPipelineState::VertexType::E_VT_VERTEXNT, m_width * m_depth, (void*)m_vertices);

	m_vertexBuffer->CreateStaticResource();

	m_vertexBuffer->RecordStatic();

	m_vertexBuffer->UploadStaticResources();

	m_vertexBuffer->ReleaseStaticCPUResource();

	//m_vertexBuffer->CreateDynamicResource();
	//m_vertexBuffer->LoadDynamicBuffer();

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

	m_indexBuffer = new CIndexBuffer();

	m_indexBuffer->Constructor(m_graphicsAdapter, m_errorLog, m_width * m_depth * 6, (void*)m_indices->m_heap);

	m_indexBuffer->CreateStaticResource();

	m_indexBuffer->RecordStatic();

	m_indexBuffer->UploadStaticResources();

	m_indexBuffer->ReleaseStaticCPUResource();

	SAFE_DELETE(m_indices);

	m_materialConstantBuffer = new CConstantBuffer();

	m_materialConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 104);

	m_materialConstantBuffer->CreateStaticResource();

	m_materialConstantBuffer->m_values->Append(ambient);
	m_materialConstantBuffer->m_values->Append(1.0f);

	m_blendConstantBuffer = new CConstantBuffer();

	m_blendConstantBuffer->Constructor(m_graphicsAdapter, m_errorLog, 128);

	m_blendConstantBuffer->CreateStaticResource();

	m_shader = new CShader();

	m_shader->Constructor(m_graphicsAdapter, m_errorLog, "Terrain");

	m_shader->SetConstantBufferCount(4);
	m_shader->SetTextureCount(11);

	m_shaderHeap = m_shader->AllocateHeap();

	m_shaderHeap->BindResource(0, m_cameraConstantBuffer->m_gpuBuffer.Get(), &m_cameraConstantBuffer->m_srvDesc);
	m_shaderHeap->BindResource(1, m_materialConstantBuffer->m_gpuBuffer.Get(), &m_materialConstantBuffer->m_srvDesc);
	m_shaderHeap->BindResource(2, m_lightConstantBuffer->m_gpuBuffer.Get(), &m_lightConstantBuffer->m_srvDesc);
	m_shaderHeap->BindResource(3, m_blendConstantBuffer->m_gpuBuffer.Get(), &m_blendConstantBuffer->m_srvDesc);

	m_shaderHeap->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

	m_overheadShader = new CShader();

	m_overheadShader->Constructor(m_graphicsAdapter, m_errorLog, "OverheadTerrain");

	m_overheadShader->SetConstantBufferCount(4);
	m_overheadShader->SetTextureCount(11);

	m_overheadShaderHeap = m_overheadShader->AllocateHeap();

	m_overheadShaderHeap->BindResource(0, m_overheadCameraConstantBuffer->m_gpuBuffer.Get(), &m_overheadCameraConstantBuffer->m_srvDesc);
	m_overheadShaderHeap->BindResource(1, m_materialConstantBuffer->m_gpuBuffer.Get(), &m_materialConstantBuffer->m_srvDesc);
	m_overheadShaderHeap->BindResource(2, m_lightConstantBuffer->m_gpuBuffer.Get(), &m_lightConstantBuffer->m_srvDesc);
	m_overheadShaderHeap->BindResource(3, m_blendConstantBuffer->m_gpuBuffer.Get(), &m_blendConstantBuffer->m_srvDesc);

	m_overheadShaderHeap->BindResource(m_shader->m_t[0], m_shadowMap->GetResource(), m_shadowMap->GetSRV());

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

			m_texture[i] = m_textureManager->Create(m_textureName, 0);

			m_shaderHeap->BindResource(m_shader->m_t[i + 1], m_texture[i]->m_texture.Get(), &m_texture[i]->m_srvDesc);

			m_overheadShaderHeap->BindResource(m_overheadShader->m_t[i + 1], m_texture[i]->m_texture.Get(), &m_texture[i]->m_srvDesc);
		}
	}

	m_blendConstantBuffer->RecordStatic();

	m_blendConstantBuffer->UploadStaticResources();

	m_blendConstantBuffer->ReleaseStaticCPUResource();

	m_materialConstantBuffer->RecordStatic();

	m_materialConstantBuffer->UploadStaticResources();

	m_materialConstantBuffer->ReleaseStaticCPUResource();

	m_isInitialized = true;
}

/*
*/
CTerrain::~CTerrain()
{
	SAFE_DELETE(m_overheadShaderHeap);
	SAFE_DELETE(m_overheadShader);
	SAFE_DELETE(m_shaderHeap);
	SAFE_DELETE(m_shader);
	SAFE_DELETE(m_materialConstantBuffer);
	SAFE_DELETE(m_materialConstantBuffer);
	SAFE_DELETE(m_blendConstantBuffer);
	SAFE_DELETE(m_indexBuffer);
	SAFE_DELETE(m_vertexBuffer);

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
	SAFE_DELETE(m_name);
}

/*
*/
void CTerrain::DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	commandList->IASetIndexBuffer(&m_indexBuffer->m_view);

	m_vertexBuffer->DrawIndexed(commandList, m_indexBuffer->m_count);
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

	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderHeap->GetBaseDescriptor());

	CTerrain::DrawGeometry(m_commandList);
}

/*
*/
void CTerrain::Record(COverhead* overhead)
{
	m_commandList->RSSetViewports(1, overhead->GetViewport());

	m_commandList->RSSetScissorRects(1, overhead->GetScissorRect());

	m_commandList->OMSetRenderTargets(1, overhead->GetRenderTarget(), false, overhead->GetDepthBuffer());

	m_commandList->SetDescriptorHeaps(1, m_overheadShaderHeap->GetDescriptorHeap());

	m_commandList->SetGraphicsRootDescriptorTable(0, m_overheadShaderHeap->GetBaseDescriptor());

	CTerrain::DrawGeometry(m_commandList);
}