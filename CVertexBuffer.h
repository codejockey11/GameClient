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

	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	int m_material;

	float m_pitch;
	float m_roll;
	float m_yaw;

	XMFLOAT4 m_position;
	XMFLOAT4 m_rotation;
	XMFLOAT4 m_scale;

	XMMATRIX m_matrixFinal;
	XMMATRIX m_matrixScaling;
	XMMATRIX m_matrixTranslation;
	XMMATRIX m_matrixYpr;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CVertexBuffer();
	~CVertexBuffer();

	void Constructor(UINT count, void* vertices);
	void Constructor(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int material, BYTE type, UINT count, void* vertices);

	void CreateResource();
	void Draw();
	void DrawIndexed(int count);
	void LoadBuffer(void* vertices);
	void Update(void* vertices);
	void UpdateRotation();
	void UpdateServer(void* vertices);

	void SetCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) { m_commandList = commandList; }

private:

	BYTE m_type;

	ComPtr<ID3D12Resource> m_vertexBuffer;

	CVertexNT* m_serverVertices;

	D3D12_HEAP_PROPERTIES m_heapProperties;
	D3D12_RANGE m_range;
	D3D12_RESOURCE_DESC m_resourceDesc;
	D3D12_VERTEX_BUFFER_VIEW m_dynamicBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_staticBufferView;

	UINT m_count;
};