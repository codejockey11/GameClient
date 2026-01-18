#include "CConstantBuffer.h"

/*
*/
CConstantBuffer::CConstantBuffer()
{
	memset(this, 0x00, sizeof(CConstantBuffer));
}

/*
*/
CConstantBuffer::~CConstantBuffer()
{
	m_gpuBuffer.Reset();
	m_cpuBuffer.Reset();

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
	SAFE_DELETE(m_values);
}

/*
*/
void CConstantBuffer::Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t size)
{
	memset(this, 0x00, sizeof(CConstantBuffer));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_size = size;

	m_values = new CHeap(m_size);

	m_needsUpload = true;
}

/*
* Limit the use of a dynamic resource to simple activities
* Not recommending a shared cpu/gpu buffer to be used
*/
void CConstantBuffer::CreateDynamicResource()
{
	m_heapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProp.CreationNodeMask = 1;
	m_heapProp.VisibleNodeMask = 1;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_desc.Alignment = 0;
	m_desc.Width = m_values->m_size;
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
		m_errorLog->WriteDXGIErrorMessage(true, "CConstantBuffer::CConstantBuffer::CreateCommittedResource::m_gpuBuffer:", m_hr);

		return;
	}

	m_gpuBuffer->SetName(L"CConstantBuffer::CConstantBuffer");

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
	m_srvDesc.Buffer.FirstElement = 0;
	m_srvDesc.Buffer.NumElements = 1;
	m_srvDesc.Buffer.StructureByteStride = m_values->m_size;
	m_srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
}

/*
*/
void CConstantBuffer::CreateStaticResource()
{
	m_commandAllocator = new CCommandAllocator(m_graphicsAdapter, m_errorLog, CGraphicsAdapter::E_BACKBUFFER_COUNT);

	m_commandList = m_commandAllocator->CreateCommandList();

	m_heapProp.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProp.CreationNodeMask = 1;
	m_heapProp.VisibleNodeMask = 1;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_desc.Alignment = 0;
	m_desc.Width = m_values->m_size;
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

	m_gpuBuffer->SetName(L"CConstantBuffer::CConstantBuffer");

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
		m_errorLog->WriteDXGIErrorMessage(true, "CConstantBuffer::CConstantBuffer::CreateCommittedResource::m_cpuBuffer:", m_hr);

		return;
	}

	m_cpuBuffer->SetName(L"CConstantBuffer::CConstantBuffer");

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
	m_srvDesc.Buffer.FirstElement = 0;
	m_srvDesc.Buffer.NumElements = 1;
	m_srvDesc.Buffer.StructureByteStride = m_values->m_size;
	m_srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
}

/*
*/
void CConstantBuffer::LoadDynamicBuffer()
{
	m_graphicsAdapter->WaitForGPU();

	m_gpuBuffer->Map(0, nullptr, (void**)&m_heapAddress);

	memcpy(m_heapAddress, m_values->m_heap, m_values->m_size);

	m_gpuBuffer->Unmap(0, nullptr);
}

/*
*/
void CConstantBuffer::RecordStatic()
{
	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator->Get(), nullptr);

	if (m_needsUpload)
	{
		m_barrier.Transition.pResource = m_gpuBuffer.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;

		m_commandList->ResourceBarrier(1, &m_barrier);

		m_cpuBuffer->Map(0, nullptr, (void**)&m_heapAddress);

		memcpy(m_heapAddress, m_values->m_heap, m_values->m_size);

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
void CConstantBuffer::ReleaseStaticCPUResource()
{
	m_cpuBuffer.Reset();

	m_commandList.Reset();

	SAFE_DELETE(m_commandAllocator);
	SAFE_DELETE(m_values);
}

/*
*/
void CConstantBuffer::Reset()
{
	m_values->Reset();

	m_needsUpload = true;
}

/*
*/
void CConstantBuffer::UploadStaticResources()
{
	m_commandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_commandList.GetAddressOf());

	m_graphicsAdapter->WaitForGPU();
}