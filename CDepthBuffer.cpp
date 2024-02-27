#include "CDepthBuffer.h"

/*
*/
CDepthBuffer::CDepthBuffer()
{
	memset(this, 0x00, sizeof(CDepthBuffer));
}

/*
*/
CDepthBuffer::CDepthBuffer(CErrorLog* errorLog, ComPtr<ID3D12Device> device, UINT width, UINT height, bool shadowMap)
{
	memset(this, 0x00, sizeof(CDepthBuffer));

	m_errorLog = errorLog;

	m_device = device;

	m_heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_heapDesc.NumDescriptors = 1;
	m_heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_heapDesc.NodeMask = 0;

	HRESULT hr = device->CreateDescriptorHeap(&m_heapDesc, IID_PPV_ARGS(&m_heap));

	if (hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CDepthBuffer::CDepthBuffer::CreateDescriptorHeap:", hr);

		return;
	}

	m_heap->SetName(L"CDepthBuffer::DescriptorHeap");

	m_handle = m_heap->GetCPUDescriptorHandleForHeapStart();


	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_desc.Alignment = 0;
	m_desc.Width = width;
	m_desc.Height = height;
	m_desc.DepthOrArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
		D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

	m_clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	m_clearValue.DepthStencil.Depth = 1.0f;
	//m_clearValue.DepthStencil.Stencil = 0;

	if (shadowMap)
	{
		m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
		m_clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	}


	hr = m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&m_clearValue,
		IID_PPV_ARGS(&m_resource));

	if (hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CDepthBuffer::CDepthBuffer::CreateCommittedResource:", hr);

		return;
	}

	if (shadowMap)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		
		m_device->CreateDepthStencilView(m_resource.Get(), &dsvDesc, m_handle);

		m_resource->SetName(L"CDepthBuffer::Shadow Map");
	}
	else
	{
		m_device->CreateDepthStencilView(m_resource.Get(), nullptr, m_handle);

		m_resource->SetName(L"CDepthBuffer::Resource");
	}

	m_barrier.Transition.pResource = m_resource.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
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
void CDepthBuffer::SetPreDrawBarrier()
{
	m_barrier.Transition.pResource = m_resource.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_READ;
}