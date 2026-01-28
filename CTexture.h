#pragma once

#include "framework.h"

#include "../GameCommon/CBmpImage.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CRawImage.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CTgaImage.h"

#include "CGraphicsAdapter.h"
#include "CPipelineState.h"
#include "CShaderBinary.h"
#include "CShaderHeap.h"
#include "CWICLoader.h"

class CTexture
{
public:

	bool m_isInitialized;

	CBmpImage* m_bmpImage;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12Resource> m_uploadHeap;

	CPipelineState* m_pipelineState;
	CRawImage* m_rawImage;
	CShaderBinary* m_computeShader;
	CShaderHeap* m_shaderHeap;
	CString* m_name;
	CTgaImage* m_tgaImage;
	CWICLoader* m_wicLoader;

	D3D12_CLEAR_VALUE m_optClear;
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_pipelineStateDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE m_currentCPUHandle;
	D3D12_DESCRIPTOR_HEAP_DESC m_computeHeapDesc;
	D3D12_GPU_DESCRIPTOR_HANDLE m_currentGPUHandle;
	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* m_subresourceFootprint;
	D3D12_RESOURCE_ALLOCATION_INFO m_allocInfo;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;
	D3D12_RESOURCE_DESC m_uploadDesc;
	D3D12_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srcTextureSRVDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
	D3D12_SUBRESOURCE_DATA m_uploadData;
	D3D12_TEXTURE_COPY_LOCATION m_copyLocationDst;
	D3D12_TEXTURE_COPY_LOCATION m_copyLocationSrc;
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_destTextureUAVDesc;

	HRESULT m_hr;

	int32_t m_dstHeight;
	int32_t m_dstWidth;
	int32_t m_height;
	int32_t m_mipCount;
	int32_t m_mipLevels;
	int32_t m_width;

	uint64_t m_uploadBufferSize;

	void* m_heapAddress;

	CTexture();
	CTexture(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CShaderBinary* shaderBinary, const char* name);
	~CTexture();

	void CreateDynamic(int32_t width, int32_t height);
	void CreateFromFilename(const char* filename, int32_t mipLevel);
	void CreateResource(BYTE* pixels, int32_t mipLevel);
	void GenerateMipMaps();
};