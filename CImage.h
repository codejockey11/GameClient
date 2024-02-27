#pragma once

#include "framework.h"

#include "CBoundBox.h"
#include "CErrorLog.h"
#include "CFloat4x4Buffer.h"
#include "CIndexBuffer.h"
#include "CDescriptorRange.h"
#include "CRootSignature.h"
#include "CShaderHeap.h"
#include "CShaderManager.h"
#include "CTexture.h"
#include "CVertexBuffer.h"
#include "CVideoDevice.h"

class CImage
{
public:

	CBoundBox* m_box;
	CFloat4x4Buffer* m_wvp;
	CFloatBuffer* m_floats;
	CIntBuffer* m_ints;
	CPipelineState* m_pipelineState;
	CDescriptorRange* m_range;
	CRootSignature* m_rootSignature;
	CShader* m_pixelShader;
	CShader* m_vertexShader;
	CShaderHeap* m_shaderHeap;
	
	CVertex2 m_position;
	CVertex2 m_scaleSize;
	CVertex2 m_size;
	
	CVertexT m_vertices[4];
	
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CVideoDevice::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CImage();
	CImage(CVideoDevice* videoDevice, CErrorLog* errorLog, CTexture* texture, CShaderManager* shaderManager, CVertex2 size, CVertex2 position);
	~CImage();

	void Draw();
	void UpdatePosition();

private:

	CIndexBuffer* m_indexBuffer;
	CTexture* m_texture;
	CVertex2 m_cellSize;
	CVertex2 m_sheetSize;
	CVertexBuffer* m_vertexBuffer;
	CVideoDevice* m_videoDevice;
	
	DWORD m_color;
	
	float m_cb;
	float m_cl;
	float m_cr;
	float m_ct;
};