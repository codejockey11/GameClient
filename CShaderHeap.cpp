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

	m_hr = m_graphicsAdapter->m_device->CreateDescriptorHeap(&m_desc, IID_PPV_ARGS(&m_heap));

	if (m_hr != S_OK)
	{
		errorLog->WriteError(true, "CShaderHeap::CShaderHeap::CreateDescriptorHeap:", m_hr);
	}

	m_CPUDescriptorHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
}

/*
*/
CShaderHeap::~CShaderHeap()
{
	m_heap.Reset();
}