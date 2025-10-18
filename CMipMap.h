#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"
#include "CShaderBinary.h"
#include "CTexture.h"
#include "CDescriptorRange.h"
#include "CPipelineState.h"
#include "CRootParameter.h"
#include "CRootSignature.h"
#include "CShaderHeap.h"

/*
* https://slindev.com/d3d12-texture-mipmap-generation/
*/

class CMipMap
{
public:

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CGraphicsAdapter* m_graphicsAdapter;
	CErrorLog* m_errorLog;
	CShaderBinary* m_computeShader;
	CDescriptorRange* m_descriptorRange;
	CPipelineState* m_pipelineState;
	CRootParameter* m_rootParameter;
	CRootSignature* m_rootSignature;
	CShaderHeap* m_shaderHeap;

	D3D12_SHADER_RESOURCE_VIEW_DESC m_srcTextureSRVDesc;
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_destTextureUAVDesc;

	D3D12_CPU_DESCRIPTOR_HANDLE m_currentCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_currentGPUHandle;

	UINT m_descriptorSize;

	uint32_t m_mipLevels;
	uint32_t m_heapSize;

	D3D12_RESOURCE_BARRIER m_barrier1;
	D3D12_RESOURCE_BARRIER m_barrier2;
	D3D12_RESOURCE_BARRIER m_barrier3;

	CMipMap();
	CMipMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog);
	~CMipMap();

	void Generate(CTexture* texture);
};