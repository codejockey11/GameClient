#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"

#include "CConstantBuffer.h"
#include "CDescriptorRange.h"
#include "CGraphicsAdapter.h"
#include "CPipelineState.h"
#include "CRootParameter.h"
#include "CRootSignature.h"
#include "CShaderBinary.h"
#include "CShaderHeap.h"

class CShader
{
public:

	enum
	{
		MAX_CONSTANT_BUFFER = 8,
		MAX_TEXTURE = 128
	};

	CConstantBuffer* m_constantBuffers;
	CDescriptorRange* m_descriptorRange;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CPipelineState* m_pipelineState;
	CRootParameter* m_rootParameter;
	CRootSignature* m_rootSignature;
	CShaderBinary* m_psBinary;
	CShaderBinary* m_vsBinary;
	CShaderHeap* m_heap;
	CShaderMaterial* m_shaderMaterial;
	CString* m_name;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptorHandle;

	int m_constantBufferCount;
	int m_textureCount;

	int* m_t;

	CShader();
	~CShader();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, const char* name);

	void AllocateConstantBuffers(int count);
	void AllocateDescriptorRange(int count);
	void AllocatePipelineState(bool depthEnable, BYTE type, D3D12_CULL_MODE cullMode, CShaderBinary* vsBinary, CShaderBinary* psBinary);
	void AllocatePipelineStateForShadowMap(BYTE type, CShaderBinary* vsBinary, CShaderBinary* psBinary);
	void AllocateRootParameter();
	void BindConstantBuffers();
	void BindResource(UINT location, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* m_srvDesc);
	void CreateConstantBuffer(int number, int size, ComPtr<ID3D12GraphicsCommandList> commandList);
	D3D12_GPU_DESCRIPTOR_HANDLE GetBaseDescriptor();
	CConstantBuffer* GetConstantBuffer(int i);
	ID3D12DescriptorHeap* const* GetDescriptorHeap();
	ID3D12PipelineState* GetPipelineState();
	ID3D12RootSignature* GetRootSignature();
	void SetTextureCount(int textureCount);
	void UpdateConstantBuffers();
};