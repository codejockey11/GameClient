#include "CShaderHeap.h"

/*
*/
CShaderHeap::CShaderHeap()
{
	memset(this, 0x00, sizeof(CShaderHeap));
}

/*
*/
CShaderHeap::CShaderHeap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t descriptorCount)
{
	memset(this, 0x00, sizeof(CShaderHeap));

	m_graphicsAdapter = graphicsAdapter;

	m_desc.NumDescriptors = descriptorCount;

	m_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_hr = m_graphicsAdapter->m_device->CreateDescriptorHeap(&m_desc, __uuidof(ID3D12DescriptorHeap), (void**)&m_heap);

	if (m_hr != S_OK)
	{
		errorLog->WriteError(true, "CShaderHeap::CShaderHeap::CreateDescriptorHeap:", m_hr);
	}

	m_heap->SetName(L"CShaderHeap::CShaderHeap");

	m_CPUDescriptorHandleForHeapStart = m_heap->GetCPUDescriptorHandleForHeapStart();

	m_GPUDescriptorHandleForHeapStart = m_heap->GetGPUDescriptorHandleForHeapStart();
}

/*
*/
CShaderHeap::~CShaderHeap()
{
	m_heap.Reset();
}

/*
*/
void CShaderHeap::BindResource(int32_t location, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
	m_CPUDescriptorHandle = m_CPUDescriptorHandleForHeapStart;

	m_CPUDescriptorHandle.ptr += ((SIZE_T)location * m_graphicsAdapter->m_descriptorHandleIncrementSize);

	m_graphicsAdapter->m_device->CreateShaderResourceView(resource, srvDesc, m_CPUDescriptorHandle);
}

/*
*/
D3D12_GPU_DESCRIPTOR_HANDLE CShaderHeap::GetBaseDescriptor()
{
	return m_heap->GetGPUDescriptorHandleForHeapStart();
}

/*
*/
ID3D12DescriptorHeap* const* CShaderHeap::GetDescriptorHeap()
{
	return m_heap.GetAddressOf();
}