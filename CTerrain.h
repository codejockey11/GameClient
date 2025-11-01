#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CEntity.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CTerrainTile.h"

#include "CCommandAllocator.h"
#include "CGraphicsAdapter.h"
#include "CIndexBuffer.h"
#include "COverhead.h"
#include "CShaderBinaryManager.h"
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
	CConstantBuffer* m_constantBuffer;
	CEntity* m_entity;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_stringBuffer[CTerrain::MAX_STRING_BUFFER];
	char m_textureName[CTerrain::MAX_TEXTURE_NAME];

	char* m_compile;
	char* m_textureValue;

	CHeapArray* m_indices;
	CIndexBuffer* m_indexBuffer;
	CLinkList<CLight>* m_lights;
	CLinkListNode<CLight>* m_lightNode;
	CLocal* m_local;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CShader* m_overheadShader;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShadowMap* m_shadowMap;
	CString* m_filename;
	CString* m_name;
	CTerrainTile* m_tile;
	CTexture* m_texture[CTerrain::MAX_TEXTURE];
	CTextureManager* m_textureManager;
	CVertexBuffer* m_vertexBuffer;
	CVertexNT* m_vertices;

	errno_t m_err;

	FILE* m_fTerrain;

	int m_height;
	int m_primSize;
	int m_scaleX;
	int m_scaleY;
	int m_size;
	int m_vertical;
	int m_width;

	CTerrain();
	CTerrain(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager,
		CEntity* entity, CShadowMap* shadowMap, const char* name);
	~CTerrain();
	
	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Record();
	void Record(COverhead* overhead);
	void SetCurrentCamera(CCamera* camera);
	void SetLights(CLinkList<CLight>* lights);
	void SetOverheadCamera(CCamera* camera);
};