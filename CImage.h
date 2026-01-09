#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CVec2f.h"
#include "../GameCommon/CVertexT.h"

#include "CBoundBox.h"
#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "CPipelineState.h"
#include "CShader.h"
#include "CShaderHeap.h"
#include "CTexture.h"
#include "CVertexBuffer.h"

class CImage
{
public:

	bool m_needsUpload;

	CBoundBox* m_box;
	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_imageWvp;
	CConstantBuffer* m_materialConstantBuffer;
	CGraphicsAdapter* m_graphicsAdapter;
	CIndexBuffer* m_indexBuffer;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CPipelineState* m_pipelineState;
	CShader* m_shader;
	CShaderHeap* m_shaderHeap;
	CShaderMaterial* m_shaderMaterial;
	CTexture* m_texture;
	CVec2f m_cellSize;
	CVec2f m_position;
	CVec2f m_scaleSize;
	CVec2f m_sheetSize;
	CVec2f m_size;
	CVertexBuffer* m_vertexBuffer;
	CVertexT m_vertices[4];

	DWORD m_color;

	int32_t m_listGroup;

	float m_cb;
	float m_cl;
	float m_cr;
	float m_ct;

	CImage();
	CImage(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CConstantBuffer* imageWvp, CPipelineState* pipelineState, int32_t listGroup, CTexture* texture,
		CVec2f position, CVec2f size);
	~CImage();

	void Move(float x, float y);
	void Record();
	void RecordConstantBuffer();
	void SetPosition(CVec2f* position);
	void UpdateVertexBuffer();
};