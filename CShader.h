#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"

#include "CGraphicsAdapter.h"
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

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CShaderBinary* m_psBinary;
	CShaderBinary* m_vsBinary;
	CShaderHeap* m_heap;
	CString* m_name;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptorHandle;

	int32_t m_constantBufferCount;
	int32_t m_textureCount;

	int32_t* m_t;

	CShader();
	~CShader();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, const char* name);

	void AllocateHeap();
	void BindResource(int32_t location, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* m_srvDesc);
	D3D12_GPU_DESCRIPTOR_HANDLE GetBaseDescriptor();
	ID3D12DescriptorHeap* const* GetDescriptorHeap();
	void SetConstantBufferCount(int32_t constantBufferCount);
	void SetTextureCount(int32_t textureCount);
};