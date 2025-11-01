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
	m_uploadHeap.Reset();

	m_buffer.Reset();

	delete m_values;
}

/*
*/
void CConstantBuffer::Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, UINT size, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	memset(this, 0x00, sizeof(CConstantBuffer));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_size = size;

	m_commandList = commandList;

	m_values = new CHeap(m_errorLog, m_size);

	m_needsUpload = true;

	m_bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_bufferDesc.Alignment = 0;
	m_bufferDesc.Width = m_values->m_size;
	m_bufferDesc.Height = 1;
	m_bufferDesc.DepthOrArraySize = 1;
	m_bufferDesc.MipLevels = 1;
	m_bufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_bufferDesc.SampleDesc.Count = 1;
	m_bufferDesc.SampleDesc.Quality = 0;
	m_bufferDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_bufferDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
	m_srvDesc.Buffer.FirstElement = 0;
	m_srvDesc.Buffer.NumElements = 1;
	m_srvDesc.Buffer.StructureByteStride = m_values->m_size;
	m_srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;

	m_bufferHeapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_bufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_bufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_bufferHeapProperties.CreationNodeMask = 1;
	m_bufferHeapProperties.VisibleNodeMask = 1;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_bufferHeapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_bufferDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr,
		IID_PPV_ARGS(&m_buffer));

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CConstantBuffer::CConstantBuffer::CreateCommittedResource::buffer:", m_hr);

		return;
	}

	m_buffer->SetName(L"CConstantBuffer::CConstantBuffer");

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_bufferDesc, 0, 1, 0, nullptr, nullptr, nullptr, &m_uploadBufferSize);

	m_allocInfo.SizeInBytes = m_uploadBufferSize;
	m_allocInfo.Alignment = 0;

	m_heapDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_heapDesc.Alignment = m_allocInfo.Alignment;
	m_heapDesc.Width = m_uploadBufferSize;
	m_heapDesc.Height = 1;
	m_heapDesc.DepthOrArraySize = 1;
	m_heapDesc.MipLevels = 1;
	m_heapDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_heapDesc.SampleDesc.Count = 1;
	m_heapDesc.SampleDesc.Quality = 0;
	m_heapDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_heapDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_heapDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_uploadHeap));

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CConstantBuffer::CConstantBuffer::CreateCommittedResource::uploadHeap:", m_hr);

		return;
	}

	m_uploadHeap->SetName(L"CConstantBuffer::CConstantBuffer");
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
void CConstantBuffer::UpdateBuffer()
{
	if (m_needsUpload)
	{
		m_barrier.Transition.pResource = m_buffer.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;

		m_commandList->ResourceBarrier(1, &m_barrier);

		m_hr = m_uploadHeap->Map(0, nullptr, &m_heapAddress);

		if (m_hr != S_OK)
		{
			m_errorLog->WriteDXGIErrorMessage(true, "CConstantBuffer::UpdateBuffer::uploadHeap->Map:", m_hr);

			return;
		}

		memcpy(m_heapAddress, m_values->m_heap, m_values->m_size);

		m_uploadHeap->Unmap(0, nullptr);

		m_commandList->CopyResource(m_buffer.Get(), m_uploadHeap.Get());

		m_barrier.Transition.pResource = m_buffer.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		m_commandList->ResourceBarrier(1, &m_barrier);

		m_needsUpload = false;
	}
}