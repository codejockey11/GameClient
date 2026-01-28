#include "CVertexBuffer.h"

/*
*/
CVertexBuffer::CVertexBuffer()
{
	memset(this, 0x00, sizeof(CVertexBuffer));
}

/*
*/
CVertexBuffer::~CVertexBuffer()
{
	m_gpuBuffer.Reset();
	m_cpuBuffer.Reset();

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
}

/*
*/
void CVertexBuffer::Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t material, BYTE type, int32_t count, void* vertices)
{
	memset(this, 0x00, sizeof(CVertexBuffer));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_type = type;

	m_count = count;

	m_material = material;

	switch (m_type)
	{
	case CPipelineState::VertexType::E_VT_VERTEX2D:
	{
		m_stride = sizeof(CVertex2D);

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXLINE:
	case CPipelineState::VertexType::E_VT_VERTEXNT:
	{
		m_stride = sizeof(CVertexNT);

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXRGBA:
	{
		m_stride = sizeof(CVertexRGBA);

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXT:
	{
		m_stride = sizeof(CVertexT);

		break;
	}
	default:
	{
		m_errorLog->WriteError(true, "CVertexBuffer::LoadBuffer:Unknown Type:%c\n", m_type);

		return;

		break;
	}
	}

	m_size = m_count * m_stride;

	m_vertices = vertices;
}

/*
*/
void CVertexBuffer::CreateDynamicResource()
{
	m_matrixYpr = XMMatrixIdentity();

	m_matrixTranslation = XMMatrixIdentity();

	m_matrixScaling = XMMatrixIdentity();

	m_matrixFinal = XMMatrixIdentity();

	// D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD The GPU will automatically upload the resource hence dynamic
	m_heapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProp.CreationNodeMask = 1;
	m_heapProp.VisibleNodeMask = 1;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_desc.Alignment = 0;
	m_desc.Width = m_size;
	m_desc.Height = 1;
	m_desc.DepthOrArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProp,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_gpuBuffer);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CVertexBuffer::CreateDynamicResource::CreateCommittedResource::m_gpuBuffer:", m_hr);

		return;
	}

	m_gpuBuffer->SetName(L"CVertexBuffer::CreateDynamicResource");

	m_vbvDesc.BufferLocation = m_gpuBuffer->GetGPUVirtualAddress();
	m_vbvDesc.StrideInBytes = m_stride;
	m_vbvDesc.SizeInBytes = m_size;
}

/*
*/
void CVertexBuffer::CreateStaticResource()
{
	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_commandList->SetName(L"CVertexBuffer::CVertexBuffer");

	m_needsUpload = true;

	// D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT Will need to record a command list to do a one time upload hence static
	m_heapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProp.CreationNodeMask = 1;
	m_heapProp.VisibleNodeMask = 1;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_desc.Alignment = 0;
	m_desc.Width = m_size;
	m_desc.Height = 1;
	m_desc.DepthOrArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProp,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_gpuBuffer);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CConstantBuffer::CConstantBuffer::CreateCommittedResource::m_gpuBuffer:", m_hr);

		return;
	}

	m_gpuBuffer->SetName(L"CVertexBuffer::CreateStaticResource");

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_desc, 0, 1, 0, nullptr, nullptr, nullptr, &m_uploadBufferSize);

	m_allocInfo.SizeInBytes = m_uploadBufferSize;
	m_allocInfo.Alignment = 0;

	m_heapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProp.CreationNodeMask = 1;
	m_heapProp.VisibleNodeMask = 1;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_desc.Alignment = m_allocInfo.Alignment;
	m_desc.Width = m_uploadBufferSize;
	m_desc.Height = 1;
	m_desc.DepthOrArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProp,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_cpuBuffer);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CVertexBuffer::CreateStaticResource::CreateCommittedResource::m_cpuBuffer:", m_hr);

		return;
	}

	m_cpuBuffer->SetName(L"CVertexBuffer::CreateStaticResource");

	m_vbvDesc.BufferLocation = m_gpuBuffer->GetGPUVirtualAddress();
	m_vbvDesc.StrideInBytes = m_stride;
	m_vbvDesc.SizeInBytes = m_size;
}

/*
*/
void CVertexBuffer::DrawIndexed(int32_t c)
{
	m_commandList->IASetVertexBuffers(0, 1, &m_vbvDesc);

	m_commandList->DrawIndexedInstanced(c, 1, 0, 0, 0);
}

/*
*/
void CVertexBuffer::DrawIndexed(ComPtr<ID3D12GraphicsCommandList> commandList, int32_t c)
{
	commandList->IASetVertexBuffers(0, 1, &m_vbvDesc);

	commandList->DrawIndexedInstanced(c, 1, 0, 0, 0);
}

/*
*/
void CVertexBuffer::LoadDynamicBuffer()
{
	void* pData = {};

	m_graphicsAdapter->WaitForGPU();

	m_gpuBuffer->Map(0, nullptr, (void**)&pData);

	memcpy(pData, m_vertices, m_size);

	m_gpuBuffer->Unmap(0, nullptr);
}

/*
*/
void CVertexBuffer::Record()
{
	m_commandList->IASetVertexBuffers(0, 1, &m_vbvDesc);

	m_commandList->DrawInstanced(m_count, 1, 0, 0);
}

/*
*/
void CVertexBuffer::Record(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vbvDesc);

	commandList->DrawInstanced(m_count, 1, 0, 0);
}

/*
*/
void CVertexBuffer::RecordStatic()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	if (m_needsUpload)
	{
		m_barrier.Transition.pResource = m_gpuBuffer.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;

		m_commandList->ResourceBarrier(1, &m_barrier);

		void* pData = {};

		m_cpuBuffer->Map(0, nullptr, (void**)&pData);

		memcpy(pData, m_vertices, m_size);

		m_cpuBuffer->Unmap(0, nullptr);

		m_commandList->CopyResource(m_gpuBuffer.Get(), m_cpuBuffer.Get());

		m_barrier.Transition.pResource = m_gpuBuffer.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		m_commandList->ResourceBarrier(1, &m_barrier);

		m_needsUpload = false;
	}
}

/*
*/
void CVertexBuffer::ReleaseStaticCPUResource()
{
	m_cpuBuffer.Reset();

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
}

/*
*/
void CVertexBuffer::UpdateDynamicBuffer()
{
	m_graphicsAdapter->WaitForGPU();

	switch (m_type)
	{
	case CPipelineState::VertexType::E_VT_VERTEXNT:
	{
		CVertexNT* pData = {};

		CVertexNT* vertex = (CVertexNT*)m_vertices;

		m_gpuBuffer->Map(0, nullptr, (void**)&pData);

		for (int32_t i = 0; i < m_count; i++)
		{
			FXMVECTOR vp = XMLoadFloat3(&vertex->m_p);

			XMVECTOR v = XMVector3TransformCoord(vp, m_matrixFinal);

			pData->m_p = XMFLOAT3(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));

			FXMVECTOR vn = XMLoadFloat3(&vertex->m_n);

			v = XMVector3TransformCoord(vn, m_matrixYpr);

			pData->m_n = XMFLOAT3(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));

			pData++;
			vertex++;
		}

		m_gpuBuffer->Unmap(0, nullptr);

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXRGBA:
	{
		CVertexRGBA* pData = {};

		CVertexRGBA* vertex = (CVertexRGBA*)m_vertices;

		m_gpuBuffer->Map(0, nullptr, (void**)&pData);

		for (int32_t i = 0; i < m_count; i++)
		{
			FXMVECTOR vp = XMLoadFloat3(&vertex->m_p);

			XMVECTOR v = XMVector3TransformCoord(vp, m_matrixYpr);

			pData->m_p = XMFLOAT3(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));

			pData++;
			vertex++;
		}

		m_gpuBuffer->Unmap(0, nullptr);

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXT:
	{
		CVertexT* pData = {};

		CVertexT* vertex = (CVertexT*)m_vertices;

		m_gpuBuffer->Map(0, nullptr, (void**)&pData);

		for (int32_t i = 0; i < m_count; i++)
		{
			FXMVECTOR vp = XMLoadFloat3(&vertex->m_p);

			XMVECTOR v = XMVector3TransformCoord(vp, m_matrixYpr);

			pData->m_p = XMFLOAT3(XMVectorGetX(v), XMVectorGetY(v), XMVectorGetZ(v));

			pData++;
			vertex++;
		}

		m_gpuBuffer->Unmap(0, nullptr);

		break;
	}
	}
}

/*
*/
void CVertexBuffer::UpdateRotation()
{
	m_pitch = m_rotation.x * (float)M_PI / 180.0f;
	m_yaw = m_rotation.y * (float)M_PI / 180.0f;
	m_roll = m_rotation.z * (float)M_PI / 180.0f;

	m_matrixScaling = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	m_matrixYpr = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, m_roll);

	m_matrixTranslation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	m_matrixFinal = XMMatrixMultiply(XMMatrixMultiply(m_matrixYpr, m_matrixScaling), m_matrixTranslation);
}

/*
*/
void CVertexBuffer::UploadStaticResources()
{
	m_commandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_commandList.GetAddressOf());

	m_graphicsAdapter->WaitForGPU();
}