#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeap.h"
#include "../GameCommon/CVertex2D.h"
#include "../GameCommon/CVertexNT.h"
#include "../GameCommon/CVertexRGBA.h"
#include "../GameCommon/CVertexT.h"

#include "CCommandAllocator.h"
#include "CGraphicsAdapter.h"
#include "CPipelineState.h"

class CVertexBuffer
{
public:

	bool m_needsUpload;

	BYTE m_type;

	CCommandAllocator* m_commandAllocator;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_gpuBuffer;
	ComPtr<ID3D12Resource> m_cpuBuffer;

	D3D12_HEAP_PROPERTIES m_heapProp;
	D3D12_RESOURCE_ALLOCATION_INFO m_allocInfo;
	D3D12_RESOURCE_BARRIER m_barrier;
	D3D12_RESOURCE_DESC m_desc;
	D3D12_SUBRESOURCE_DATA m_uploadData;
	D3D12_VERTEX_BUFFER_VIEW m_vbvDesc;

	float m_pitch;
	float m_roll;
	float m_yaw;

	HRESULT m_hr;

	int32_t m_count;
	int32_t m_material;
	int32_t m_size;
	int32_t m_stride;

	uint64_t m_uploadBufferSize;

	void* m_vertices;

	XMFLOAT4 m_position;
	XMFLOAT4 m_rotation;
	XMFLOAT4 m_scale;

	XMMATRIX m_matrixFinal;
	XMMATRIX m_matrixScaling;
	XMMATRIX m_matrixTranslation;
	XMMATRIX m_matrixYpr;

	CVertexBuffer();
	~CVertexBuffer();

	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t material, BYTE type, int32_t count, void* vertices);
	void CreateDynamicResource();
	void CreateStaticResource();
	void DrawIndexed(int32_t count);
	void DrawIndexed(ComPtr<ID3D12GraphicsCommandList> commandList, int32_t count);
	void LoadDynamicBuffer();
	void Record();
	void Record(ComPtr<ID3D12GraphicsCommandList> commandList);
	void RecordStatic();
	void ReleaseStaticCPUResource();
	void UpdateDynamicBuffer();
	void UpdateRotation();
	void UploadStaticResources();
};