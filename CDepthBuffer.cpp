#include "CDepthBuffer.h"

/*
*/
CDepthBuffer::CDepthBuffer()
{
	memset(this, 0x00, sizeof(CDepthBuffer));
}

/*
*/
CDepthBuffer::CDepthBuffer(CErrorLog* errorLog, ComPtr<ID3D12Device> device, int32_t numDescriptors, int32_t width, int32_t height)
{
	memset(this, 0x00, sizeof(CDepthBuffer));

	m_errorLog = errorLog;

	m_device = device;

	m_width = width;

	m_height = height;

	m_heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_heapDesc.NumDescriptors = numDescriptors;
	m_heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_heapDesc.NodeMask = 0;

	m_hr = device->CreateDescriptorHeap(&m_heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_heap);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CDepthBuffer::CDepthBuffer::CreateDescriptorHeap:", m_hr);

		return;
	}

	m_heap->SetName(L"CDepthBuffer::DescriptorHeap");

	m_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
}

/*
*/
CDepthBuffer::~CDepthBuffer()
{
	m_resource.Reset();

	m_heap.Reset();
}

/*
*/
void CDepthBuffer::Create()
{
	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_desc.Alignment = 0;
	m_desc.Width = m_width;
	m_desc.Height = m_height;
	m_desc.DepthOrArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	m_clearValue.DepthStencil.Depth = 1.0f;
	m_clearValue.DepthStencil.Stencil = 0;

	m_hr = m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&m_clearValue,
		__uuidof(ID3D12Resource),
		(void**)&m_resource);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CDepthBuffer::CDepthBuffer::CreateCommittedResource:", m_hr);

		return;
	}

	m_depthStencilViewDesc.Flags = D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;
	m_depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	m_depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	m_depthStencilViewDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(m_resource.Get(), &m_depthStencilViewDesc, m_handle);

	m_resource->SetName(L"CDepthBuffer::Resource");
}

/*
*/
void CDepthBuffer::CreateShadowMap()
{
	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_desc.Alignment = 0;
	m_desc.Width = m_width;
	m_desc.Height = m_height;
	m_desc.DepthOrArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	m_clearValue.DepthStencil.Depth = 1.0f;
	m_clearValue.DepthStencil.Stencil = 0;

	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};

	formatSupport.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;

	m_hr = m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));

	if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1::D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
	{
		m_hr = m_device->CreateCommittedResource(&m_heapProperties,
			D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&m_desc,
			D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
			&m_clearValue,
			__uuidof(ID3D12Resource),
			(void**)&m_resource);

		m_depthStencilViewDesc.Flags = D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;
		m_depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
		m_depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		m_depthStencilViewDesc.Texture2D.MipSlice = 0;

		m_device->CreateDepthStencilView(m_resource.Get(), &m_depthStencilViewDesc, m_handle);

		m_resource->SetName(L"CDepthBuffer::ShadowMap");

		m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
		m_srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
		m_srvDesc.Texture2D.MostDetailedMip = 0;
		m_srvDesc.Texture2D.MipLevels = 1;
		m_srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		m_srvDesc.Texture2D.PlaneSlice = 0;

		return;
	}

	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;

	m_hr = m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		&m_clearValue,
		__uuidof(ID3D12Resource),
		(void**)&m_resource);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CDepthBuffer::CreateShadowMap::CreateCommittedResource:", m_hr);

		return;
	}

	m_depthStencilViewDesc.Flags = D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;
	m_depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	m_depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_depthStencilViewDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(m_resource.Get(), &m_depthStencilViewDesc, m_handle);

	m_resource->SetName(L"CDepthBuffer::ShadowMap");

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	m_srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	m_srvDesc.Texture2D.MostDetailedMip = 0;
	m_srvDesc.Texture2D.MipLevels = 1;
	m_srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_srvDesc.Texture2D.PlaneSlice = 0;
}

/*
*/
void CDepthBuffer::SetPostDrawBarrier()
{
	m_barrier.Transition.pResource = m_resource.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

/*
*/
void CDepthBuffer::SetPreDrawBarrier()
{
	m_barrier.Transition.pResource = m_resource.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
}