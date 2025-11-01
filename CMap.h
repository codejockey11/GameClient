#pragma once

#include "framework.h"

#include "../GameCommon/CEntity.h"
#include "../GameCommon/CErrorLog.h"
#include "../GameCommon/CLinkList.h"
#include "../GameCommon/CLocal.h"
#include "../GameCommon/CShaderMaterial.h"
#include "../GameCommon/CString.h"
#include "../GameCommon/CVec3f.h"

#include "CCommandAllocator.h"
#include "CGraphicsAdapter.h"
#include "COverhead.h"
#include "CShader.h"
#include "CShaderBinaryManager.h"
#include "CShadowMap.h"
#include "CSoundDevice.h"
#include "CSoundManager.h"
#include "CTerrain.h"
#include "CTextureManager.h"
#include "CVertexBuffer.h"
#include "CVisibility.h"
#include "CWavefrontManager.h"

class CMap
{
public:

	CCommandAllocator* m_commandAllocator;
	CConstantBuffer* m_constantBuffer;
	CEntity* m_entity;
	CErrorLog* m_errorLog;
	CGraphicsAdapter* m_graphicsAdapter;

	char m_key[CKeyValue::MAX_KEY];
	char m_value[CKeyValue::MAX_VALUE];
	char* m_objectName;

	CLinkList<CLight>* m_lights;
	CLinkList<CObject>* m_sectors;
	CLinkList<CVertexBuffer>* m_vertexBuffers;
	CLinkListNode<CLight>* m_lightNode;
	CLinkListNode<CVertexBuffer>* m_vertexBufferNode;
	CLight* m_light;
	CLocal* m_local;
	CObject* m_object;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CShader* m_overheadShader;
	CShader* m_shader;
	CShaderBinary* m_ps;
	CShaderBinary* m_vs;
	CShaderBinaryManager* m_shaderBinaryManager;
	CShaderMaterial* m_shaderMaterial;
	CShadowMap* m_shadowMap;
	CSoundDevice* m_soundDevice;
	CSoundManager* m_soundManager;
	CString* m_filename;
	CString* m_name;
	CTerrain* m_terrain;
	CTexture* m_texture;
	CTextureManager* m_textureManager;
	CVec3f m_vec3f;
	CVec3i m_index;
	CVec3i m_mapSize;
	CVertexBuffer* m_vertexBuffer;
	CVertexNT* m_vertices;
	CVisibility* m_visibility;
	CWavefront* m_wavefront;
	CWavefrontManager* m_wavefrontManager;
	CWavLoader* m_wavLoader;

	errno_t m_err;

	FILE* m_fMap;

	float m_float;

	int m_entityCount;
	int m_keyValueCount;
	int m_materialNumber;
	int m_maxEntityCount;
	int m_numberOfMaterials;
	int m_sectorSize;
	int m_vertexBufferCount;
	int m_verticesCount;

	XMFLOAT4 m_xmfloat4;

	CMap();
	CMap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, CLocal* local, CSoundDevice* soundDevice, CWavefrontManager* wavefrontManager,
		CTextureManager* textureManager, CShaderBinaryManager* shaderBinaryManager, CSoundManager* m_soundManager, char* name, CShadowMap* shadowMap);
	~CMap();

	void DrawGeometry(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Record();
	void Record(COverhead* overhead);
	void SetCurrentCamera(CCamera* camera);
	void SetOverheadCamera(CCamera* camera);
};