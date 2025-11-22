#include "CIndexBuffer.h"

/*
*/
CIndexBuffer::CIndexBuffer()
{
	memset(this, 0x00, sizeof(CIndexBuffer));
}

/*
*/
CIndexBuffer::CIndexBuffer(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t count, void* indicies)
{
	memset(this, 0x00, sizeof(CIndexBuffer));

	m_count = count;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_resourceDesc.Alignment = 0;
	m_resourceDesc.Width = m_count * sizeof(uint16_t);
	m_resourceDesc.Height = 1;
	m_resourceDesc.DepthOrArraySize = 1;
	m_resourceDesc.MipLevels = 1;
	m_resourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_resourceDesc.SampleDesc.Count = 1;
	m_resourceDesc.SampleDesc.Quality = 0;
	m_resourceDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_resourceDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_hr = graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer));

	if (m_hr != S_OK)
	{
		errorLog->WriteDXGIErrorMessage(true, "CIndexBuffer::CreateResource::CreateCommittedResource:", m_hr);

		return;
	}

	m_indexBuffer->SetName(L"CIndexBuffer::Resource");

	m_view.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_view.Format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
	m_view.SizeInBytes = m_count * sizeof(uint16_t);

	m_indexBuffer->Map(0, nullptr, &m_heapAddress);

	memcpy(m_heapAddress, indicies, m_count * sizeof(uint16_t));

	m_indexBuffer->Unmap(0, nullptr);
}

/*
*/
CIndexBuffer::~CIndexBuffer()
{
	m_indexBuffer.Reset();
}

/*
*/
void CIndexBuffer::Record()
{
	m_commandList->IASetIndexBuffer(&m_view);
}

/*
*/
void CIndexBuffer::SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	m_commandList = commandList;
}