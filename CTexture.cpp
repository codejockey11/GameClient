#include "CTexture.h"

//Union used for shader constants
struct DWParam
{
	DWParam(FLOAT f) : Float(f) {}
	DWParam(UINT u) : Uint(u) {}

	void operator= (FLOAT f) { Float = f; }
	void operator= (UINT u) { Uint = u; }

	union
	{
		FLOAT Float;
		UINT Uint;
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
CTexture::CTexture(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinary* shaderBinary, const char* name)
{
	memset(this, 0x00, sizeof(CTexture));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_local = local;

	m_computeShader = shaderBinary;

	m_name = new CString(name);

	if (m_local)
	{
		m_filename = new CString(m_local->m_installPath->m_text);

		m_filename->Append(m_local->m_assetDirectory->m_text);
		
		m_filename->Append(m_name->m_text);
	}
}

/*
*/
CTexture::~CTexture()
{
	delete m_shaderHeap;
	delete m_pipelineState;
	delete m_rootSignature;
	delete m_rootParameter;
	delete m_descriptorRange;

	m_uploadHeap.Reset();

	m_texture.Reset();

	m_commandList.Reset();

	m_commandAllocator.Reset();

	delete m_filename;
	delete m_name;
}

/*
*/
void CTexture::CreateDynamic(UINT width, UINT height)
{
	m_width = width;
	m_height = height;

	m_desc.MipLevels = 1;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	m_desc.Width = m_width;
	m_desc.Height = m_height;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	m_desc.DepthOrArraySize = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_texture));

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
		IID_PPV_ARGS(&m_uploadHeap));

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::CreateCommittedResource::textureUploadHeap:", m_hr);

		return;
	}

	m_uploadHeap->SetName(L"CTexture::UploadHeap");

	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.Format = m_desc.Format;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	m_srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;
}

/*
*/
void CTexture::CreateFromFilename()
{
	m_graphicsAdapter->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

	m_graphicsAdapter->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	if (m_name->Search(".bmp"))
	{
		m_bmpImage = new CBmpImage(m_filename->m_text);

		if (m_bmpImage->m_isInitialized)
		{
			m_width = m_bmpImage->m_bmapInfo.bmiHeader.biWidth;
			m_height = m_bmpImage->m_bmapInfo.bmiHeader.biHeight;

			CTexture::CreateResource(m_bmpImage->m_pixels32);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", m_name->m_text);
		}

		delete m_bmpImage;

		return;
	}
	else if (m_name->Search(".tga"))
	{
		m_tgaImage = new CTgaImage(m_filename->m_text);

		if (m_tgaImage->m_isInitialized)
		{
			m_width = m_tgaImage->m_header->m_width;
			m_height = m_tgaImage->m_header->m_height;

			CTexture::CreateResource(m_tgaImage->m_pixels32);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", m_name->m_text);
		}

		delete m_tgaImage;
	}
	else if (m_name->Search(".idat"))
	{
		m_rawImage = new CRawImage(m_filename->m_text);

		if (m_rawImage->m_isInitialized)
		{
			m_width = m_rawImage->m_width;
			m_height = m_rawImage->m_height;

			CTexture::CreateResource(m_rawImage->m_pixels32);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", m_name->m_text);
		}

		delete m_rawImage;
	}
	else
	{
		m_wicLoader = new CWICLoader(m_filename->m_text);

		if (m_wicLoader->m_isInitialized)
		{
			m_width = m_wicLoader->m_width;
			m_height = m_wicLoader->m_height;

			CTexture::CreateResource(m_wicLoader->m_pixels32);

			m_isInitialized = true;
		}
		else
		{
			m_errorLog->WriteError(true, "CTexture::CTexture::Not Created:%s\n", m_name->m_text);
		}

		delete m_wicLoader;
	}
}

/*
*/
void CTexture::CreateResource(BYTE* pixels)
{
	m_desc.MipLevels = 0;
	m_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	m_desc.Width = m_width;
	m_desc.Height = m_height;
	m_desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	m_desc.DepthOrArraySize = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;
	m_desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	m_heapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	m_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	m_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	m_heapProperties.CreationNodeMask = 1;
	m_heapProperties.VisibleNodeMask = 1;

	m_hr = m_graphicsAdapter->m_device->CreateCommittedResource(&m_heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&m_desc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_texture));

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
		IID_PPV_ARGS(&m_uploadHeap));

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::CreateCommittedResource::textureUploadHeap:", m_hr);

		return;
	}

	m_uploadHeap->SetName(L"CTexture::UploadHeap");

	m_uploadData.pData = pixels;
	m_uploadData.RowPitch = (LONG_PTR)m_width * 4;
	m_uploadData.SlicePitch = m_uploadData.RowPitch * m_height;

	m_hr = m_uploadHeap->Map(0, nullptr, &m_heapAddress);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::CreateResource::textureUploadHeap->Map:", m_hr);

		return;
	}

	memcpy(m_heapAddress, m_uploadData.pData, m_uploadData.SlicePitch);

	m_uploadHeap->Unmap(0, nullptr);

	UINT64 MemToAlloc = sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64);

	if (MemToAlloc > SIZE_MAX)
	{
		m_errorLog->WriteError(true, "CTexture::CreateResource::MemToAlloc > SIZE_MAX\n");

		return;
	}

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pMem = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)HeapAlloc(GetProcessHeap(), 0, MemToAlloc);

	if (pMem == nullptr)
	{
		m_errorLog->WriteError(true, "CTexture::CreateResource::HeapAlloc::pMem == nullptr\n");

		return;
	}

	UINT64 RequiredSize = 0;

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_desc, 0, 1, 0, pMem, &m_height, (UINT64*)&m_uploadData.RowPitch, &RequiredSize);

	D3D12_TEXTURE_COPY_LOCATION Src = {};

	Src.pResource = m_uploadHeap.Get();
	Src.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	Src.PlacedFootprint = *pMem;

	D3D12_TEXTURE_COPY_LOCATION Dst = {};

	Dst.pResource = m_texture.Get();
	Dst.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	Dst.SubresourceIndex = 0;

	m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);

	if (!HeapFree(GetProcessHeap(), 0, pMem))
	{
		m_errorLog->WriteError(true, "CTexture::CreateResource::HeapFree:false\n");

		return;
	}

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
}

/*
*/
void CTexture::GenerateMipMaps()
{
	m_descriptorRange = new CDescriptorRange(2);

	m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);
	m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1);

	m_rootParameter = new CRootParameter(3);

	m_rootParameter->InitDescriptorTable(0, 1, &m_descriptorRange->m_range[0]);
	m_rootParameter->InitDescriptorTable(1, 1, &m_descriptorRange->m_range[1]);
	
	m_rootParameter->Init32BitConstant(2, 2);

	m_rootSignature = new CRootSignature(m_graphicsAdapter, m_errorLog);

	m_rootSignature->Create(m_rootParameter->m_count, m_rootParameter->m_rootParameter);

	m_pipelineState = new CPipelineState(m_graphicsAdapter, m_errorLog);

	m_pipelineState->CreateCompute(m_rootSignature, m_computeShader->m_shader);

	m_srcTextureSRVDesc = {};

	m_srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	m_destTextureUAVDesc = {};

	m_destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	m_mipLevels = m_texture->GetDesc().MipLevels;

	m_heapSize = m_mipLevels - 1;

	m_shaderHeap = new CShaderHeap(m_graphicsAdapter, m_errorLog, 2 * m_heapSize);

	m_currentCPUHandle = m_shaderHeap->m_heap->GetCPUDescriptorHandleForHeapStart();
	m_currentGPUHandle = m_shaderHeap->m_heap->GetGPUDescriptorHandleForHeapStart();

	m_descriptorSize = m_graphicsAdapter->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_commandList->SetComputeRootSignature(m_rootSignature->m_signature.Get());
	m_commandList->SetPipelineState(m_pipelineState->m_pipelineState.Get());
	m_commandList->SetDescriptorHeaps(1, m_shaderHeap->m_heap.GetAddressOf());

	m_barrier = {};

	m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrier.Transition.pResource = m_texture.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	m_commandList->ResourceBarrier(1, &m_barrier);

	for (uint32_t topMip = 0; topMip < m_mipLevels - 1; topMip++)
	{
		//Get mipmap dimensions
		uint32_t dstWidth = max((UINT)m_width >> (topMip + 1), 1);
		uint32_t dstHeight = max(m_height >> (topMip + 1), 1);

		// Create shader resource view for the source texture in the descriptor heap
		// Texture2D<float4> SrcTexture : register(t0);
		m_srcTextureSRVDesc.Format = m_graphicsAdapter->m_backBufferFormat;
		m_srcTextureSRVDesc.Texture2D.MipLevels = 1;
		m_srcTextureSRVDesc.Texture2D.MostDetailedMip = topMip;

		m_graphicsAdapter->m_device->CreateShaderResourceView(m_texture.Get(), &m_srcTextureSRVDesc, m_currentCPUHandle);

		m_currentCPUHandle.ptr += m_descriptorSize;

		// Create unordered access view for the destination texture in the descriptor heap
		// RWTexture2D<float4> DstTexture : register(u0);
		m_destTextureUAVDesc.Format = m_graphicsAdapter->m_backBufferFormat;
		m_destTextureUAVDesc.Texture2D.MipSlice = topMip + 1;

		m_graphicsAdapter->m_device->CreateUnorderedAccessView(m_texture.Get(), nullptr, &m_destTextureUAVDesc, m_currentCPUHandle);

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

		//Dispatch the compute shader with one thread per 32x32 pixels
		m_commandList->Dispatch(max(dstWidth / 32, 1u), max(dstHeight / 32, 1u), 1);

		//Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
		m_barrier = {};

		m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		m_barrier.UAV.pResource = m_texture.Get();
		m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		m_commandList->ResourceBarrier(1, &m_barrier);
	}

	m_barrier = {};

	m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrier.Transition.pResource = m_texture.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_commandList->ResourceBarrier(1, &m_barrier);
}

/*
*/
void CTexture::UpdateTextureResource(void* pixels)
{
	m_uploadData.pData = pixels;
	m_uploadData.RowPitch = (LONG_PTR)m_width * 4;
	m_uploadData.SlicePitch = m_uploadData.RowPitch * m_height;

	m_hr = m_uploadHeap->Map(0, nullptr, &m_heapAddress);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CTexture::UpdateTextureResource::textureUploadHeap->Map:", m_hr);

		return;
	}

	memcpy(m_heapAddress, m_uploadData.pData, m_uploadData.SlicePitch);

	m_uploadHeap->Unmap(0, nullptr);

	UINT64 RequiredSize = 0;

	UINT64 MemToAlloc = sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64);

	if (MemToAlloc > SIZE_MAX)
	{
		m_errorLog->WriteError(true, "CTexture::UpdateTextureResource::MemToAlloc > SIZE_MAX\n");

		return;
	}

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pMem = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)HeapAlloc(GetProcessHeap(), 0, MemToAlloc);

	if (pMem == nullptr)
	{
		m_errorLog->WriteError(true, "CTexture::UpdateTextureResource::HeapAlloc:pMem == nullptr\n");

		return;
	}

	m_graphicsAdapter->m_device->GetCopyableFootprints(&m_desc, 0, 1, 0, pMem, &m_height, (UINT64*)&m_uploadData.RowPitch, &RequiredSize);

	D3D12_TEXTURE_COPY_LOCATION Src = {};

	Src.pResource = m_uploadHeap.Get();
	Src.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	Src.PlacedFootprint = *pMem;

	D3D12_TEXTURE_COPY_LOCATION Dst = {};

	Dst.pResource = m_texture.Get();
	Dst.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	Dst.SubresourceIndex = 0;

	m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);

	if (!HeapFree(GetProcessHeap(), 0, pMem))
	{
		m_errorLog->WriteError(true, "CTexture::UpdateTextureResource::HeapFree:false\n");

		return;
	}

	m_barrier.Transition.pResource = m_texture.Get();
	m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
	m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_commandList->ResourceBarrier(1, &m_barrier);
}