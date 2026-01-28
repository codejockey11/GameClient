#include "CRenderTarget.h"

/*
*/
CRenderTarget::CRenderTarget()
{
	memset(this, 0x00, sizeof(CRenderTarget));
}

/*
*/
CRenderTarget::CRenderTarget(CErrorLog* errorLog, ComPtr<ID3D12Device> device)
{
	memset(this, 0x00, sizeof(CRenderTarget));

	m_errorLog = errorLog;

	m_device = device;

	m_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
	m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	m_heapDesc.NumDescriptors = 1;
	m_heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	m_hr = m_device->CreateDescriptorHeap(&m_heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_heap);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CRenderTarget::CRenderTarget::CreateDescriptorHeap:", m_hr);

		return;
	}

	m_heap->SetName(L"CRenderTarget::Heap");

	m_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
}

/*
*/
CRenderTarget::~CRenderTarget()
{
	m_wrappedBackBuffer.Reset();
	m_surface.Reset();
	m_d2dRenderTarget.Reset();
	m_resource.Reset();
	m_heap.Reset();
}

/*
*/
void CRenderTarget::AdditionalTarget(int32_t width, int32_t height, DXGI_FORMAT format)
{
	m_desc.MipLevels = 1;
	m_desc.Format = format;
	m_desc.Width = width;
	m_desc.Height = height;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	m_desc.DepthOrArraySize = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_clearValue.Format = format;

	m_hr = m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,
		&m_clearValue,
		__uuidof(ID3D12Resource),
		(void**)&m_resource);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CRenderTarget::AdditionalTarget::CreateCommittedResource::resource:", m_hr);

		return;
	}

	m_resource->SetName(L"CRenderTarget::AdditionalRenderTarget");

	m_device->CreateRenderTargetView(m_resource.Get(), nullptr, m_handle);
}

/*
*/
void CRenderTarget::SetPostDrawBarrier()
{
	m_barrier.Transition.pResource = m_resource.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
}

/*
*/
void CRenderTarget::SetPreDrawBarrier()
{
	m_barrier.Transition.pResource = m_resource.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
}

/*
*/
void CRenderTarget::SwapChainTarget(ComPtr<IDXGISwapChain3> swapChain, int32_t bufferNbr)
{
	m_hr = swapChain->GetBuffer(bufferNbr, __uuidof(ID3D12Resource), (void**)&m_resource);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CRenderTarget::SwapChainTarget::GetBuffer:", m_hr);

		return;
	}

	m_device->CreateRenderTargetView(m_resource.Get(), nullptr, m_handle);

	m_resource->SetName(L"CRenderTarget::SwapChainTarget");
}

/*
*/
#ifdef D3D11on12
void CRenderTarget::WrapResource(float dpi, ComPtr<ID3D11On12Device> d3d11On12Device, ComPtr<ID2D1DeviceContext1> d2dDeviceContext)
{
	m_d3d11On12Device = d3d11On12Device;

	m_d2dDeviceContext = d2dDeviceContext;

	m_d3d11Flags =
	{
		D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET
	};

	m_hr = m_d3d11On12Device->CreateWrappedResource(m_resource.Get(), &m_d3d11Flags, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT, __uuidof(ID3D11Resource), (void**)&m_wrappedBackBuffer);

	m_hr = m_wrappedBackBuffer->QueryInterface(m_surface.GetAddressOf());

	m_bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS::D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS::D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi, dpi);

	if (m_surface)
	{
		m_hr = m_d2dDeviceContext->CreateBitmapFromDxgiSurface(m_surface.Get(), &m_bitmapProperties, m_d2dRenderTarget.GetAddressOf());

		if (m_d2dRenderTarget)
		{
			m_rtSize = m_d2dRenderTarget->GetSize();
		}
	}
}
#endif