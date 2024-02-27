#pragma once

#include "framework.h"

#include "CCamera.h"
#include "CCommandListBundle.h"
#include "CErrorLog.h"
#include "CFloat4x4Buffer.h"
#include "CFloatBuffer.h"
#include "CHeapArray.h"
#include "CIndexBuffer.h"
#include "CLinkList.h"
#include "CMaterial.h"
#include "COverhead.h"
#include "CPipelineState.h"
#include "CDescriptorRange.h"
#include "CRootSignature.h"
#include "CShaderHeap.h"
#include "CShaderManager.h"
#include "CShadowMap.h"
#include "CTerrainTile.h"
#include "CTextureManager.h"
#include "CVertexBuffer.h"
#include "CVideoDevice.h"

class CTerrain
{
public:

	CFloat4x4Buffer* m_wvp;
	CFloatBuffer* m_floats;
	CFloatBuffer* m_overheadFloats;
	CFloat4x4Buffer* m_overheadWvp;
	CIntBuffer* m_ints;
	CIntBuffer* m_overheadInts;
	CPipelineState* m_pipelineState;
	CDescriptorRange* m_descriptorRange;
	CRootSignature* m_rootSignature;
	CShader* m_pixelShader;
	CShader* m_vertexShader;
	CShaderHeap* m_shaderHeap;
	CShaderHeap* m_overheadHeap;
	CShadowMap* m_shadowMap;
	CString* m_name;
	CVideoDevice* m_videoDevice;
	
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CVideoDevice::E_BACKBUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	
	UINT m_height;
	UINT m_primSize;
	UINT m_width;

	CTerrain();
	CTerrain(CVideoDevice* videoDevice, CErrorLog* errorLog, CTextureManager* textureManager, CShaderManager* shaderManager, const char* filename);
	~CTerrain();
	
	void DrawGeometry();
	void Record();
	void Record(COverhead* overhead);
	void SetCurrentCamera(CCamera* camera);
	void SetOverheadCamera(CCamera* camera);

private:

	CIndexBuffer* m_indexBuffer;
	CTerrainTile* m_tile;
	CTexture* m_texture0;
	CTexture* m_texture1;
	CTexture* m_texture2;
	CTexture* m_texture3;
	CTexture* m_texture4;
	CTexture* m_texture5;
	CVertexBuffer* m_vertexBuffer;
	
	int m_size;
};