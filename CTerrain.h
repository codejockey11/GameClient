#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CEntity.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CList.h"
#include "../GameCommon/CTerrainTile.h"

#include "CCommandAllocator.h"
#include "CConstantBuffer.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "COverhead.h"
#include "CPipelineState.h"
#include "CShaderHeap.h"
#include "CShadowMap.h"
#include "CTextureManager.h"
#include "CVertexBuffer.h"

class CTerrain
{
public:

	enum
	{
		SHADOWMAP_SLOT = 3,
		TEXTURE_SLOT_START = 4,
		MAX_TEXTURE = 6,
		MAX_TEXTURE_NAME = 128,
		MAX_STRING_BUFFER = 32
	};

	bool m_isInitialized;

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_blendConstantBuffer;
	CConstantBuffer* m_cameraConstantBuffer;
	CConstantBuffer* m_lightConstantBuffer;
	CConstantBuffer* m_materialConstantBuffer;
	CConstantBuffer* m_overheadCameraConstantBuffer;
	CEntity* m_entity;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_stringBuffer[CTerrain::MAX_STRING_BUFFER];
	char m_textureName[CTerrain::MAX_TEXTURE_NAME];

	char* m_textureValue;

	CHeapArray* m_indices;
	CIndexBuffer* m_indexBuffer;
	CLight* m_light;
	CList* m_lights;
	CListNode* m_node;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CPipelineState* m_pipelineState;
	CShader* m_overheadShader;
	CShader* m_shader;
	CShaderHeap* m_overheadShaderHeap;
	CShaderHeap* m_shaderHeap;
	CShadowMap* m_shadowMap;
	CString* m_name;
	CTerrainTile* m_tile;
	CTexture* m_texture[CTerrain::MAX_TEXTURE];
	CTextureManager* m_textureManager;
	CVertexBuffer* m_vertexBuffer;
	CVertexNT* m_vertices;

	errno_t m_err;

	FILE* m_fTerrain;

	int32_t m_listGroup;
	int32_t m_depth;
	int32_t m_height;
	int32_t m_primSize;
	int32_t m_scaleX;
	int32_t m_scaleY;
	int32_t m_size;
	int32_t m_width;

	CTerrain();
	CTerrain(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CTextureManager* textureManager, CPipelineState* pipelineState, int32_t listGroup,
		CEntity* entity, CShadowMap* shadowMap, CConstantBuffer* camera, CConstantBuffer* overheadCamera, CConstantBuffer* light, CVec3f* ambient,
		const char* name, const char* filename);
	~CTerrain();

	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Record();
	void Record(COverhead* overhead);
};