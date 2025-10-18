#include "CMipMap.h"

/*
*/
CMipMap::CMipMap()
{
	memset(this, 0x00, sizeof(CMipMap));
}

/*
*/
CMipMap::CMipMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CMipMap));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;


	m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->Close();

	// Can't generate mip maps past 6.4 as it's probably a needed driver upgrade which I can't do with the hardware I am working with
	m_computeShader = new CShaderBinary(m_errorLog, "shaders/mipMap.hlsl", "CSMain", "cs_6_4", D3D_SHADER_MODEL::D3D_SHADER_MODEL_6_4);

	m_descriptorRange = new CDescriptorRange(2);

	m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);
	m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1);

	m_rootParameter = new CRootParameter(3);

	m_rootParameter->Init32BitConstant(2, 2);
	m_rootParameter->InitDescriptorTable(0, 1, &m_descriptorRange->m_range[0]);
	m_rootParameter->InitDescriptorTable(1, 1, &m_descriptorRange->m_range[1]);

	m_rootSignature = new CRootSignature(m_graphicsAdapter, m_errorLog);

	m_rootSignature->Create(m_rootParameter->m_count, m_rootParameter->m_rootParameter);

	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog);

	m_pipelineState->CreateCompute(m_rootSignature, m_computeShader->m_shader);


	m_srcTextureSRVDesc = {};

	m_srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	m_destTextureUAVDesc = {};

	m_destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
}

/*
*/
CMipMap::~CMipMap()
{
	delete m_shaderHeap;
	delete m_pipelineState;
	delete m_rootSignature;
	delete m_rootParameter;
	delete m_descriptorRange;
	delete m_computeShader;

	m_commandList.Reset();
	m_commandAllocator.Reset();
}

/*
*/
void CMipMap::Generate(CTexture* texture)
{
	m_mipLevels = texture->m_texture->GetDesc().MipLevels;

	m_heapSize = m_mipLevels - 1;

	m_shaderHeap = new CShaderHeap(m_graphicsAdapter, m_errorLog, 2 * m_heapSize);

	m_currentCPUHandle = m_shaderHeap->m_heap->GetCPUDescriptorHandleForHeapStart();
	m_currentGPUHandle = m_shaderHeap->m_heap->GetGPUDescriptorHandleForHeapStart();

	m_descriptorSize = m_graphicsAdapter->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_commandList->SetComputeRootSignature(m_rootSignature->m_signature.Get());
	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());
	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->m_heap.GetAddressOf());

	m_barrier1 = {};

	m_barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrier1.Transition.pResource = texture->m_texture.Get();
	m_barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	m_commandList->ResourceBarrier(1, &m_barrier1);

	for (uint32_t TopMip = 0; TopMip < m_mipLevels - 1; TopMip++)
	{
		//Get mipmap dimensions
		uint32_t dstWidth = max((UINT)texture->m_width >> (TopMip + 1), 1);
		uint32_t dstHeight = max(texture->m_height >> (TopMip + 1), 1);

		
		
		
		// Create shader resource view for the source texture in the descriptor heap
		// Texture2D<float4> SrcTexture : register(t0);
		m_srcTextureSRVDesc.Format = m_graphicsAdapter->m_backBufferFormat;
		m_srcTextureSRVDesc.Texture2D.MipLevels = 1;
		m_srcTextureSRVDesc.Texture2D.MostDetailedMip = TopMip;
		
		m_graphicsAdapter->m_device->CreateShaderResourceView(texture->m_texture.Get(), &m_srcTextureSRVDesc, m_currentCPUHandle);

		m_currentCPUHandle.ptr += m_descriptorSize;

		
		
		
		
		// Create unordered access view for the destination texture in the descriptor heap
		// RWTexture2D<float4> DstTexture : register(u0);
		m_destTextureUAVDesc.Format = m_graphicsAdapter->m_backBufferFormat;
		m_destTextureUAVDesc.Texture2D.MipSlice = TopMip + 1;
		
		m_graphicsAdapter->m_device->CreateUnorderedAccessView(texture->m_texture.Get(), nullptr, &m_destTextureUAVDesc, m_currentCPUHandle);

		m_currentCPUHandle.ptr += m_descriptorSize;

		
		
		
		
		// Pass the source and destination texture views to the shader via descriptor tables
		m_commandList->SetComputeRootDescriptorTable(0, m_currentGPUHandle);

		m_currentGPUHandle.ptr += m_descriptorSize;

		m_commandList->SetComputeRootDescriptorTable(1, m_currentGPUHandle);

		m_currentGPUHandle.ptr += m_descriptorSize;

		
		
		
		
		// Pass the destination texture pixel size to the shader as constants
		// cbuffer CB : register(b0)
		m_commandList->SetComputeRoot32BitConstant(2, DWParam(1.0f / dstWidth).Uint, 0);
		m_commandList->SetComputeRoot32BitConstant(2, DWParam(1.0f / dstHeight).Uint, 1);

		
		
		//Dispatch the compute shader with one thread per 8x8 pixels
		m_commandList->Dispatch(max(dstWidth / 8, 1u), max(dstHeight / 8, 1u), 1);

		
		
		
		
		
		//Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
		m_barrier2 = {};

		m_barrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		m_barrier2.UAV.pResource = texture->m_texture.Get();
		m_barrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		m_barrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		
		m_commandList->ResourceBarrier(1, &m_barrier2);
	}

	m_barrier3 = {};

	m_barrier3.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrier3.Transition.pResource = texture->m_texture.Get();
	m_barrier3.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	m_barrier3.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_commandList->ResourceBarrier(1, &m_barrier3);

	m_commandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_commandList.GetAddressOf());

	m_graphicsAdapter->WaitForGPU();
}