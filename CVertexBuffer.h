#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CVertex2D.h"
#include "../GameCommon/CVertexNT.h"
#include "../GameCommon/CVertexRGBA.h"
#include "../GameCommon/CVertexT.h"

#include "CGraphicsAdapter.h"
#include "CPipelineState.h"

class CVertexBuffer
{
public:

	BYTE m_type;

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12Resource> m_vertexBuffer;

	CVertexNT* m_serverVertices;

	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_RANGE m_range;
	D3D12_RESOURCE_DESC m_resourceDesc;
	D3D12_VERTEX_BUFFER_VIEW m_dynamicBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_staticBufferView;

	float m_pitch;
	float m_roll;
	float m_yaw;

	HRESULT m_hr;

	int32_t m_material;

	int32_t m_count;

	void* m_heapAddress;

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
	void Constructor(int32_t count, void* vertices);
	void CreateResource();
	void DrawIndexed(int32_t count);
	void LoadBuffer(void* vertices);
	void Record();
	void SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Update(void* vertices);
	void UpdateRotation();
	void UpdateServer(void* vertices);
};