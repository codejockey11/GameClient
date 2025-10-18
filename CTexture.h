#pragma once

#include "framework.h"

#include "../GameCommon/CBmpImage.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CRawImage.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CTgaImage.h"

#include "CDescriptorRange.h"
#include "CGraphicsAdapter.h"
#include "CPipelineState.h"
#include "CRootParameter.h"
#include "CRootSignature.h"
#include "CShaderBinary.h"
#include "CShaderHeap.h"
#include "CWICLoader.h"

class CTexture
{
public:

	CBmpImage* m_bmpImage;
	CDescriptorRange* m_descriptorRange;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CLocal* m_local;
	CPipelineState* m_pipelineState;
	CRawImage* m_rawImage;
	CRootParameter* m_rootParameter;
	CRootSignature* m_rootSignature;
	CShaderBinary* m_computeShader;
	CShaderHeap* m_shaderHeap;
	CString* m_filename;
	CString* m_name;
	CTgaImage* m_tgaImage;
	CWICLoader* m_wicLoader;

	D3D12_CLEAR_VALUE m_optClear;
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_pipelineStateDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE m_currentCPUHandle;
	D3D12_DESCRIPTOR_HEAP_DESC m_computeHeapDesc;
	D3D12_GPU_DESCRIPTOR_HANDLE m_currentGPUHandle;
	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_RESOURCE_ALLOCATION_INFO m_allocInfo;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;
	D3D12_RESOURCE_DESC m_uploadDesc;
	D3D12_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srcTextureSRVDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
	D3D12_SUBRESOURCE_DATA m_uploadData;
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_destTextureUAVDesc;

	bool m_isInitialized;

	HRESULT m_hr;

	UINT m_descriptorSize;
	UINT m_height;

	uint32_t m_heapSize;
	uint32_t m_mipLevels;

	UINT64 m_uploadBufferSize;
	UINT64 m_width;

	void* m_pData;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12Resource> m_uploadHeap;

	CTexture();
	CTexture(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CShaderBinary* shaderBinary, const char* name);
	~CTexture();

	void CreateDynamic(UINT width, UINT height);
	void CreateForShadowMap();
	void CreateResource(BYTE* pixels);
	void CreateFromFilename();
	void GenerateMipMaps();
	void SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) { m_commandList = commandList; }
	void UpdateTextureResource(void* bytes);
};