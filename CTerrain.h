#pragma once

#include "framework.h"

#include "../GameCommon/CCamera.h"
#include "../GameCommon/CEntity.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CHeapArray.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CTerrainTile.h"

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

	CEntity* m_entity;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;
	CHeapArray* m_indices;
	CIndexBuffer* m_indexBuffer;
	CLocal* m_local;
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

	bool m_isInitialized;

	char* m_compile;
	
	errno_t m_err;

	FILE* m_fTerrain;
	
	int m_height;
	int m_primSize;
	int m_size;
	int m_vertical;
	int m_width;

	int m_scaleX;
	int m_scaleY;

	char m_textureName[CTerrain::MAX_TEXTURE_NAME];
	char m_stringBuffer[CTerrain::MAX_STRING_BUFFER];

	char* m_textureValue;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[CGraphicsAdapter::E_BACKBUFFER_COUNT];

	CTerrain();
	CTerrain(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager,
		CEntity* entity, CShadowMap* shadowMap, const char* name);
	~CTerrain();
	
	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Record();
	void Record(COverhead* overhead);
	void SetCurrentCamera(CCamera* camera);
	void SetOverheadCamera(CCamera* camera);
};