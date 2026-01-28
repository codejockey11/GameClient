#include "CTexture.h"

//Union used for shader constants
struct DWParam
{
	DWParam(FLOAT f) : Float(f) {}
	DWParam(int32_t u) : Uint32_t(u) {}

	void operator= (FLOAT f) { Float = f; }
	void operator= (int32_t u) { Uint32_t = u; }

	union
	{
		FLOAT Float;
		int32_t Uint32_t;
	};
};

/*
*/
CTexture::CTexture()
{
	memset(this, 0x00, sizeof(CTexture));
}

/*
*/
CTexture::CTexture(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinary* shaderBinary, const char* name)
{
	memset(this, 0x00, sizeof(CTexture));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_computeShader = shaderBinary;

	m_name = new CString(name);

	m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)&m_commandAllocator);

	m_commandAllocator->SetName(m_name->GetWide());

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr,
		__uuidof(ID3D12GraphicsCommandList), (void**)&m_commandList);

	m_commandList->SetName(m_name->GetWide());
}

/*
*/
CTexture::~CTexture()
{
	SAFE_DELETE(m_shaderHeap);
	SAFE_DELETE(m_pipelineState);

	m_uploadHeap.Reset();

	m_texture.Reset();

	m_commandList.Reset();

	m_commandAllocator.Reset();

	SAFE_DELETE(m_name);
}

/*
*/
void CTexture::CreateDynamic(int32_t width, int32_t height)
{
	m_width = width;
	m_height = height;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	m_desc.Width = m_width;
	m_desc.Height = m_height;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	m_desc.DepthOrArraySize = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_texture);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::CreateCommittedResource::texture:", m_hr);

		return;
	}

	m_uploadBufferSize = 0;

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_desc, 0, 1, 0, nullptr, nullptr, nullptr, &m_uploadBufferSize);

	m_allocInfo.SizeInBytes = m_uploadBufferSize;
	m_allocInfo.Alignment = 0;

	m_uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_uploadDesc.Alignment = m_allocInfo.Alignment;
	m_uploadDesc.Width = m_uploadBufferSize;
	m_uploadDesc.Height = 1;
	m_uploadDesc.DepthOrArraySize = 1;
	m_uploadDesc.MipLevels = 1;
	m_uploadDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_uploadDesc.SampleDesc.Count = 1;
	m_uploadDesc.SampleDesc.Quality = 0;
	m_uploadDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_uploadDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_uploadDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_uploadHeap);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::CreateCommittedResource::textureUploadHeap:", m_hr);

		return;
	}

	m_uploadHeap->SetName(m_name->GetWide());

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.Format = m_desc.Format;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	m_srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;
}

/*
*/
void CTexture::CreateFromFilename(const char* filename, int32_t mipLevel)
{
	if (strstr(filename, ".bmp") != 0)
	{
		m_bmpImage = new CBmpImage(filename);

		if (m_bmpImage->m_isInitialized)
		{
			m_width = m_bmpImage->m_bmapInfo.bmiHeader.biWidth;
			m_height = m_bmpImage->m_bmapInfo.bmiHeader.biHeight;

			CTexture::CreateResource(m_bmpImage->m_pixels32, mipLevel);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", filename);
		}

		SAFE_DELETE(m_bmpImage);
	}
	else if (strstr(filename, ".tga") != 0)
	{
		m_tgaImage = new CTgaImage(filename);

		if (m_tgaImage->m_isInitialized)
		{
			m_width = m_tgaImage->m_header->m_width;
			m_height = m_tgaImage->m_header->m_height;

			CTexture::CreateResource(m_tgaImage->m_pixels32, mipLevel);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", filename);
		}

		SAFE_DELETE(m_tgaImage);
	}
	else if (strstr(filename, ".idat") != 0)
	{
		m_rawImage = new CRawImage(filename);

		if (m_rawImage->m_isInitialized)
		{
			m_width = m_rawImage->m_width;
			m_height = m_rawImage->m_height;

			CTexture::CreateResource(m_rawImage->m_pixels32, mipLevel);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", filename);
		}

		SAFE_DELETE(m_rawImage);
	}
	else
	{
		m_wicLoader = new CWICLoader(filename);

		if (m_wicLoader->m_isInitialized)
		{
			m_width = m_wicLoader->m_width;
			m_height = m_wicLoader->m_height;

			CTexture::CreateResource(m_wicLoader->m_pixels32, mipLevel);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", filename);
		}

		SAFE_DELETE(m_wicLoader);
	}
}

/*
*/
void CTexture::CreateResource(BYTE* pixels, int32_t mipLevel)
{
	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_desc.MipLevels = (uint16_t)mipLevel;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	m_desc.Width = m_width;
	m_desc.Height = m_height;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	m_desc.DepthOrArraySize = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_texture);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::CreateCommittedResource::texture:", m_hr);

		return;
	}
	
	m_texture->SetName(m_name->GetWide());

	m_uploadBufferSize = 0;

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_desc, 0, 1, 0, nullptr, nullptr, nullptr, &m_uploadBufferSize);

	m_allocInfo.SizeInBytes = m_uploadBufferSize;
	m_allocInfo.Alignment = 0;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	m_uploadDesc.Alignment = m_allocInfo.Alignment;
	m_uploadDesc.Width = m_uploadBufferSize;
	m_uploadDesc.Height = 1;
	m_uploadDesc.DepthOrArraySize = 1;
	m_uploadDesc.MipLevels = 1;
	m_uploadDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_uploadDesc.SampleDesc.Count = 1;
	m_uploadDesc.SampleDesc.Quality = 0;
	m_uploadDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	m_uploadDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_uploadDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_uploadHeap);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::CreateCommittedResource::textureUploadHeap:", m_hr);

		return;
	}

	m_uploadData.pData = pixels;
	m_uploadData.RowPitch = m_width * 4;
	m_uploadData.SlicePitch = m_uploadData.RowPitch * m_height;

	m_uploadHeap->Map(0, nullptr, &m_heapAddress);

	memcpy(m_heapAddress, m_uploadData.pData, m_uploadData.SlicePitch);

	m_uploadHeap->Unmap(0, nullptr);

	
	m_subresourceFootprint = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)HeapAlloc(GetProcessHeap(), 0, m_uploadData.SlicePitch);

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_desc, 0, 1, 0, m_subresourceFootprint, (uint32_t*)&m_height, (uint64_t*)&m_uploadData.RowPitch, nullptr);

	m_copyLocationSrc.pResource = m_uploadHeap.Get();
	m_copyLocationSrc.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	m_copyLocationSrc.PlacedFootprint = *m_subresourceFootprint;

	m_copyLocationDst.pResource = m_texture.Get();
	m_copyLocationDst.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	m_copyLocationDst.SubresourceIndex = 0;

	m_commandList->CopyTextureRegion(&m_copyLocationDst, 0, 0, 0, &m_copyLocationSrc, nullptr);

	HeapFree(GetProcessHeap(), 0, m_subresourceFootprint);


	m_barrier.Transition.pResource = m_texture.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_commandList->ResourceBarrier(1, &m_barrier);

	if (m_texture->GetDesc().MipLevels > 1)
	{
		CTexture::GenerateMipMaps();
	}

	m_commandList->Close();

	m_graphicsAdapter->m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_commandList.GetAddressOf());

	m_graphicsAdapter->WaitForGPU();

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.Format = m_desc.Format;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	m_srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;

	SAFE_DELETE(m_shaderHeap);
	SAFE_DELETE(m_pipelineState);

	m_uploadHeap.Reset();

	m_commandList.Reset();

	m_commandAllocator.Reset();
}

/*
*/
void CTexture::GenerateMipMaps()
{
	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog);

	m_pipelineState->SetComputeShader(m_computeShader);

	m_pipelineState->CreateCompute();

	m_srcTextureSRVDesc = {};

	m_srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;

	m_destTextureUAVDesc = {};

	m_destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;

	m_mipLevels = m_texture->GetDesc().MipLevels;

	m_mipCount = m_mipLevels - 1;

	m_shaderHeap = new CShaderHeap(m_graphicsAdapter, m_errorLog, 3 * m_mipCount);

	m_currentCPUHandle = m_shaderHeap->m_CPUDescriptorHandleForHeapStart;
	m_currentGPUHandle = m_shaderHeap->m_GPUDescriptorHandleForHeapStart;

	m_commandList->SetComputeRootSignature(m_pipelineState->m_rootSignature->m_signature.Get());
	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());
	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->m_heap.GetAddressOf());

	m_barrier = {};

	m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrier.Transition.pResource = m_texture.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	m_commandList->ResourceBarrier(1, &m_barrier);

	for (int32_t topMip = 0; topMip < m_mipCount; topMip++)
	{
		//Get mipmap dimensions
		m_dstWidth = max(m_width >> (topMip + 1), 1);
		m_dstHeight = max(m_height >> (topMip + 1), 1);

		// Create shader resource view for the source texture in the descriptor heap
		// Texture2D<float4> SrcTexture : register(t0);
		m_srcTextureSRVDesc.Format = m_graphicsAdapter->m_backBufferFormat;
		m_srcTextureSRVDesc.Texture2D.MipLevels = 1;
		m_srcTextureSRVDesc.Texture2D.MostDetailedMip = topMip;

		m_graphicsAdapter->m_device->CreateShaderResourceView(m_texture.Get(), &m_srcTextureSRVDesc, m_currentCPUHandle);

		m_currentCPUHandle.ptr += m_graphicsAdapter->m_descriptorHandleIncrementSize;

		// Create unordered access view for the destination texture in the descriptor heap
		// RWTexture2D<float4> DstTexture : register(u0);
		m_destTextureUAVDesc.Format = m_graphicsAdapter->m_backBufferFormat;
		m_destTextureUAVDesc.Texture2D.MipSlice = topMip + 1;

		m_graphicsAdapter->m_device->CreateUnorderedAccessView(m_texture.Get(), nullptr, &m_destTextureUAVDesc, m_currentCPUHandle);

		m_currentCPUHandle.ptr += m_graphicsAdapter->m_descriptorHandleIncrementSize;

		// Pass the source and destination texture views to the shader via descriptor tables
		m_commandList->SetComputeRootDescriptorTable(0, m_currentGPUHandle);

		m_currentGPUHandle.ptr += m_graphicsAdapter->m_descriptorHandleIncrementSize;

		m_commandList->SetComputeRootDescriptorTable(1, m_currentGPUHandle);

		m_currentGPUHandle.ptr += m_graphicsAdapter->m_descriptorHandleIncrementSize;

		// Pass the destination texture pixel size to the shader as constants
		// cbuffer CB : register(b0)
		m_commandList->SetComputeRoot32BitConstant(2, DWParam(1.0f / m_dstWidth).Uint32_t, 0);
		m_commandList->SetComputeRoot32BitConstant(2, DWParam(1.0f / m_dstHeight).Uint32_t, 1);

		//Dispatch the compute shader
		// [numthreads(32, 32, 1)]
		m_commandList->Dispatch(max(m_dstWidth / 8, 1u), max(m_dstHeight / 8, 1u), 1);

		//Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
		m_barrier = {};

		m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_UAV;
		m_barrier.UAV.pResource = m_texture.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		m_commandList->ResourceBarrier(1, &m_barrier);
	}

	m_barrier = {};

	m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrier.Transition.pResource = m_texture.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_commandList->ResourceBarrier(1, &m_barrier);
}